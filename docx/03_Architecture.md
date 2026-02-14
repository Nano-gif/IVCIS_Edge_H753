# IVCIS 高速入口智能视觉终端 - 架构设计

## 1. 系统架构总览

```
┌─────────────────────────────────────────────────────────────┐
│                      云端服务器 (UDP)                         │
│          • AI推理 (车型/车牌识别)                          │
│          • 返回违规标识 / 舵机调角指令                      │
└─────────────────────────────────────────────────────────────┘
                              ▲         │
                       上传  │         │  返回结果
                       JPEG │         │  + 运维指令
                              │         ▼
┌─────────────────────────────────────────────────────────────┐
│                    STM32H753ZI 边缘节点                      │
│  ┌─────────┐  ┌─────────┐  ┌─────────────────┐           │
│  │ OV5640  │──│ Motion  │──│   Net_Client    │           │
│  │ (JPEG)  │  │ Detect  │  │ (UDP Send/Recv) │           │
│  └────┬────┘  └─────────┘  └────────┬────────┘           │
│       │                            │                       │
│  ┌────┴────┐  ┌──────────┐  ┌────┴─────────┐            │
│  │  DCMI   │  │ AutoExp  │  │  Alarm_Handler│            │
│  │  + DMA  │  │+ Quality │  │  (GPIO/LED)   │            │
│  └─────────┘  └──────────┘  └──────────────┘             │
│                                                            │
│  ┌──────────┐  ┌─────────────┐                            │
│  │  Servo   │  │Power_Manager│                            │
│  │  (PWM)   │  │ (三级功耗)    │                            │
│  └──────────┘  └─────────────┘                            │
└─────────────────────────────────────────────────────────────┘
```

> **架构原则**：边缘端不做 AI 推理，所有车型/车牌识别在云端完成。边缘端负责粗筛（帧差法）+ 数据采集 + 质量控制。

## 2. 模块职责定义

| 模块名称 | 职责 | 对外接口 |
|----------|------|----------|
| **Vision_Pipeline** | OV5640 初始化、JPEG/灰度模式切换、DMA 采集 | `Vision_Init()`, `Vision_GetJpegFrame()`, `Vision_SetMode()` |
| **Motion_Detect** | 帧差法运动检测（纯数学，无AI） | `Motion_Init()`, `Motion_Detect()` |
| **Net_Client** | LwIP UDP 初始化、JPEG 上报、接收云端结果和运维指令 | `Net_Init()`, `Net_SendImage()`, `Net_RecvResult()` |
| **Alarm_Handler** | GPIO 驱动、声光报警控制 | `Alarm_Trigger()`, `Alarm_Stop()` |
| **Power_Manager** | 三级功耗模式管理、帧差驱动唤醒 | `Power_SetMode()`, `Power_GetMode()` |
| **Auto_Exposure** | 图像亮度分析 + 模糊度评估、SCCB 寄存器调整 | `AutoExp_Analyze()`, `AutoExp_Adjust()` |
| **Servo_Control** | 舵机 PWM 驱动、远程调角 | `Servo_Init()`, `Servo_SetAngle()` |
| **Debug_Logger** | 分级日志输出 | `DBG_INFO()`, `DBG_WARN()`, `DBG_ERROR()` |

> **已删除模块**：`AI_Inference`（AI 在云端完成）、`JPEG_Encoder`（使用 OV5640 内置 JPEG）

## 3. 数据流设计

### 3.1 设计决策：使用 OV5640 内置 JPEG

由于 **AI 推理在云端完成**，边缘端无需 RGB 原图，因此直接使用 OV5640 内置 JPEG 编码器：

| 考量 | OV5640 JPEG 输出 | RGB565 + STM32 JPEG |
|------|------------------|---------------------|
| **复杂度** | **简单** ✅ | 复杂 |
| **CPU 占用** | **几乎为零** ✅ | 需要 DMA/MDMA |
| **云端 AI** | **直接可用** ✅ | 直接可用 |
| **边缘 AI** | 需解码（但我们不需要） | 直接可用 |

### 3.2 数据流图

```
OV5640 ──DCMI──▶ Frame_Buffer (D2 SRAM)
                      │
    ┌─────────────────┼──────────────────────┐
    │                 │                      │
    ▼                 ▼                      ▼
Motion_Detect    AutoExp_Analyze        Net_Client
(灰度帧差)      (亮度/模糊度)          (JPEG上传)
    │                 │                      │
    │                 ▼                      ▼
    │           OV5640 寄存器调整        云端 AI 推理
    │                                        │
    ▼                                        ▼
Power_Manager                       ┌───────────────┐
(功耗模式切换)                      │ 返回: 违规标识 │
                                    │       调角指令 │
                                    └───────┬───────┘
                                            │
                    ┌───────────────────────┤
                    ▼                       ▼
              Alarm_Handler          Servo_Control
              (声光报警)            (远程调角)
```

### 3.3 帧差法数据源方案

> **方案 C (已确认)**：帧差唤醒模式下，OV5640 通过 SCCB 切换为 **160x120 灰度输出**。检测到运动后，切回 **JPEG 模式**。切换延迟 ~100ms，对"有无车辆"的检测粒度完全可接受。

| 模式 | OV5640 输出 | 帧率 | 用途 |
|------|------------|------|------|
| 全速/轻活跃 | JPEG | 10/2 fps | 图像采集 + 上传 |
| 帧差唤醒 | 灰度 160x120 | 1 fps | 帧差法运动检测 |

### 3.4 图像质量闭环控制

```
         ┌───────────────────────────────────┐
         │    Image Quality Closed Loop      │
         │                                   │
  Frame ─┤  ① 亮度统计 (arm_mean_f32)       │
         │       ↓                           │
         │  ② 模糊度 (Laplacian Variance)   │
         │       ↓                           │
         │  ③ 决策:                          │
         │     过暗 → SCCB 增加曝光/增益     │
         │     过曝 → SCCB 减少曝光/增益     │
         │     模糊 → 标记 LOW_QUALITY flag  │
         └───────────────────────────────────┘
```

> **发挥 H753 特性**：使用 Cortex-M7 FPU + CMSIS-DSP 库加速统计计算。

## 4. 内存布局

| 区域 | 地址范围 | 用途 | 属性 |
|------|----------|------|------|
| **D1 AXI-SRAM** | 0x24000000 - 0x2407FFFF | 主堆栈、FreeRTOS Heap | Cacheable |
| **D2 SRAM** | 0x30000000 - 0x30047FFF | DCMI JPEG 缓冲、ETH 描述符、LwIP Heap | Non-cacheable |
| **D3 SRAM** | 0x38000000 - 0x3800FFFF | 低功耗保留 | Non-cacheable |
| **FLASH** | 0x08000000 - 0x081FFFFF | 代码 (2MB) | - |

## 5. FreeRTOS 任务设计

| 任务名 | 优先级 | 栈大小 | 职责 | 备注 |
|--------|--------|--------|------|------|
| **Task_Camera** | 40 (高) | 2048 | 视觉初始化、帧差运动检测、模式切换 (JPEG/Gray) | 核心业务流 |
| **Task_Net** | 24 (低) | 2048 | LwIP 协议栈处理、UDP 数据收发 | 协议驱动 |
| **Task_AI** | 32 (中) | 4096 | (已挂起) 云端 AI 模式下边缘端无需此任务 | 保留作为扩展桩 |

> [!NOTE]
> 为减少任务切换开销，已将**帧差法运动检测**逻辑直接合并至 `Task_Camera`。

## 6. 接口定义 (Header Files)

### 6.1 Vision_Pipeline.h
```c
typedef enum {
    VISION_MODE_JPEG,  // JPEG 压缩输出 (全速/轻活跃)
    VISION_MODE_GRAY   // 160x120 灰度 (帧差唤醒)
} VisionMode_t;

int8_t   Vision_Init(void);
void     Vision_SetMode(VisionMode_t mode);  // 切换 OV5640 输出模式
void     Vision_CaptureStart(void);          // 启动单帧采集 (同步)
uint8_t  Vision_IsFrameReady(void);
uint8_t* Vision_GetFrameBuffer(void);
uint32_t Vision_GetFrameSize(void);
```

### 6.2 Net_Client.h
```c
int8_t  Net_Client_Init(void);
void    Net_Client_SendImage(uint8_t* pData, uint32_t len, uint32_t frame_id);
bool    Net_Client_RecvResult(ViolationResult_t* result);
void    Net_Client_Diagnostic(void);
```

> **零拷贝实现**：`Net_Client_SendImage()` 使用 `PBUF_ROM` 类型。

### 6.3 Alarm_Handler.h
```c
void Alarm_Init(void);
void Alarm_Trigger(AlarmType_t type);
void Alarm_Stop(void);
```

### 6.4 Motion_Detect.h
```c
void Motion_Init(uint16_t width, uint16_t height);
bool Motion_Detect(uint8_t* current_frame);
uint32_t Motion_GetDiff(void);
```

### 6.5 Power_Manager.h
```c
typedef enum {
    POWER_MODE_FULL,        // JPEG @ 10fps + 上传云端
    POWER_MODE_LIGHT,       // JPEG @ 2fps + 上传云端
    POWER_MODE_MOTION_ONLY  // 灰度 160x120 @ 1fps + 帧差法
} PowerMode_t;

void Power_Init(void);
void Power_SetMode(PowerMode_t mode);
PowerMode_t Power_GetMode(void);
void Power_UpdateIdleTime(bool has_motion);
```

### 6.6 Auto_Exposure.h
```c
typedef struct {
    float brightness;   // 平均亮度 [0-255]
    float sharpness;    // Laplacian 方差 (越高越清晰)
    bool  is_low_quality;
} ImageQuality_t;

void           AutoExp_Init(void);
ImageQuality_t AutoExp_Analyze(uint8_t* frame, uint32_t size);
void           AutoExp_Adjust(ImageQuality_t* quality);
```

### 6.7 Servo_Control.h
```c
void    Servo_Init(void);
void    Servo_SetAngle(uint16_t angle_deg);  // 0~180
uint16_t Servo_GetAngle(void);
```

## 7. 通信协议

### 7.1 UDP 上报格式 (MCU → 云端)
```c
typedef struct {
    uint32_t magic;         // 0x49564349 ("IVCI")
    uint32_t frame_id;
    uint8_t  flags;         // bit0: low_quality, bit1: alarm_active
    uint8_t  reserved[3];
    uint32_t jpeg_size;
    uint8_t  jpeg_data[];   // 变长
} IVCIS_Packet_t;
```

### 7.2 云端下行指令格式 (云端 → MCU)
```c
typedef struct {
    uint32_t magic;         // 0x49564352 ("IVCR")
    uint8_t  cmd_type;      // 0x01=违规结果, 0x02=舵机调角
    uint8_t  reserved[3];
    union {
        struct { uint8_t is_violation; char plate[16]; } result;
        struct { uint16_t angle_deg; } servo;
    } payload;
} IVCIS_Command_t;
```

### 7.3 网络配置
| 参数 | 值 |
|------|-----|
| 本机 IP | 192.168.1.10 |
| 目标 IP | 192.168.1.100 |
| 上行端口 | 8080 |
| 下行端口 | 8081 |
| MTU | 1500 |

## 8. 引脚映射 (基于 NUCLEO-H753ZI 数据手册)

### 8.1 ETH RMII (板载 LAN8742A, 不可更改)

| 信号 | MCU Pin |
|------|---------|
| RMII_REF_CLK | PA1 |
| RMII_MDIO | PA2 |
| RMII_MDC | PC1 |
| RMII_CRS_DV | PA7 |
| RMII_RXD0 | PC4 |
| RMII_RXD1 | PC5 |
| RMII_TX_EN | PG11 |
| RMII_TXD0 | PG13 |
| RMII_TXD1 | PB13 |

### 8.2 DCMI (通过转接板连接 OV5640)

| 信号 | MCU Pin | Morpho 位置 |
|------|---------|-------------|
| DCMI_HSYNC | PA4 | CN11-32 |
| DCMI_VSYNC | PB7 | CN11-21 |
| DCMI_PIXCLK | PA6 | CN12-13 |
| DCMI_D0 | PC6 | CN12-4 |
| DCMI_D1 | PC7 | CN12-19 |
| DCMI_D2 | PC8 | CN12-2 |
| DCMI_D3 | PC9 | CN12-1 |
| DCMI_D4 | PE4 | CN11-48 |
| DCMI_D5 | PD3 | CN11-40 |
| DCMI_D6 | PE5 | CN11-50 |
| DCMI_D7 | PE6 | CN11-62 |

> **注意**：DCMI_D3 选用 PC9 而非 PG11（被 ETH 占用）或 PE1（被 LD2 占用）。

### 8.3 I2C (SCCB - OV5640 配置)

| 信号 | MCU Pin | Morpho 位置 |
|------|---------|-------------|
| I2C1_SCL | PB8 | CN12-3 |
| I2C1_SDA | PB9 | CN12-5 |

### 8.4 VCP 调试串口 (板载, 不可更改)

| 信号 | MCU Pin |
|------|---------|
| USART3_TX | PD8 |
| USART3_RX | PD9 |

### 8.5 LED 指示

| LED | MCU Pin | 用途 |
|-----|---------|------|
| LD1 (Green) | PB0 | 系统状态 |
| LD2 (Yellow) | PE1 | 网络/运动指示 |
| LD3 (Red) | PB14 | 违规报警 |

### 8.6 舵机 PWM

| 信号 | MCU Pin | 定时器 | Morpho 位置 |
|------|---------|--------|-------------|
| Servo PWM | PD15 (TIM4_CH4) | TIM4 | CN7-18 (D9) |

### 8.7 用户按键

| 信号 | MCU Pin |
|------|---------|
| B1 USER | PC13 |

---

**文档版本**: v2.2  
**更新日期**: 2026-02-14  
**维护者**: IVCIS Team  
**变更记录**:  
- v2.2: 实施非阻塞 RTOS 任务集成；合并 AI/运动检测逻辑至 Camera 任务；更新接口列表。
- v2.1: 同步 Vision_Pipeline 灰度模式实现 (`VISION_MODE_GRAY`), 新增 `Vision_CaptureStart` API
