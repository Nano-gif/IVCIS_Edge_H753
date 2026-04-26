# IVCIS 工业智能相机 - 架构设计

> **V2 更新**: 生产者-消费者并行架构, Netconn API, Transport 抽象层, RS485 Modbus RTU
> **V2.5 规划**: 引入毫米波雷达外设, 用雷达目标列表替代视觉帧差作为主触发源, 支持恶劣天气下的抓拍与融合判断

## 1. 系统架构总览

```
┌──────────────────────────────────────────────────────────┐
│                     云端 / 上位机                          │
│     • AI 推理 (车型/车牌识别)                             │
│     • MQTT 设备管理 (远期)                                │
│     • 识别结果可下发相机或直达 PLC (Modbus TCP 等)         │
└──────────────┬──────────────────────┬────────────────────┘
               │ UDP (图传+指令)       │ 可直连 PLC
               ▼                      ▼
┌──────────────────────────────────────────────────────────┐
│                   STM32H753ZI 工业智能相机                 │
│                                                           │
│  ┌────────────────────────────────────────────────────┐  │
│  │              Application Layer                     │  │
│  │  Vision_Pipeline │ Radar_Manager │ Power_Manager   │  │
│  │  Auto_Exposure   │ Alarm_Handler │ Motion_Detect*  │  │
│  └──────────┬───────────────────────────────────────┘  │
│             │ FrameDesc_t (osMessageQueue)              │
│  ┌──────────┴───────────────────────────────────────┐  │
│  │        双控制源: UDP 指令 ∥ Modbus 寄存器写入      │  │
│  │          → 同一执行 API (Last-Write-Wins)         │  │
│  └──────┬──────────────────────────┬────────────────┘  │
│         │                          │                    │
│  ┌──────┴──────┐           ┌───────┴────────┐          │
│  │Transport_ETH│           │Transport_RS485 │          │
│  │(Netconn UDP)│           │(Modbus RTU)    │          │
│  └──────┬──────┘           └───────┬────────┘          │
│         │                          │                    │
│  ┌──────┴──────┐           ┌───────┴────────┐          │
│  │ LAN8742A    │           │ MAX485 + UART  │          │
│  │ RMII 100M   │           │ 半双工 RS485   │          │
│  └─────────────┘           └────────────────┘          │
└──────────────────────────────────────────────────────────┘
         │                          │
    ┌────┴────┐              ┌──────┴──────┐
    │ 云端/PC  │              │  PLC / HMI  │
    └─────────┘              └─────────────┘
```

> **架构原则**:
> - 边缘端不做复杂 AI 推理和雷达原始点云处理, 识别和复杂融合在云端/上位机完成
> - 采集与传输通过消息队列解耦, 并行流水线执行
> - 通信通道可插拔: 以太网 (数据面) + RS485 (控制面)
> - **双控制源**: 云端 UDP 指令和 RS485 Modbus 均可控制报警/舵机/模式切换, 部署时二选一或并存, 执行层接口统一
> - LwIP 使用 Netconn API 保证线程安全
> - 雷达外设输出目标列表, H753 只做目标级融合、状态机和协议转发

## 2. 模块职责定义

| 模块名称 | 职责 | 对外接口 |
|----------|------|----------|
| **Vision_Pipeline** | OV5640 JPEG/灰度模式管理, 双缓冲切换 | `Vision_Init()`, `Vision_SetMode()`, `Vision_CaptureStart()` |
| **vision_capture** | DCMI+DMA 底层采集, force_stop | `Vision_ForceStop()`, `Vision_CaptureOne()` |
| **Motion_Detect** | 灰度帧差法运动检测 (纯整数), 雷达方案中降级为 fallback | `Motion_Init()`, `Motion_Detect()` |
| **Radar_Manager** | 雷达目标列表解析、目标区判断、触发摘要生成 | `Radar_Init()`, `Radar_Poll()`, `Radar_GetSummary()` |
| **Net_Client** | Netconn UDP 初始化, 零拷贝分片发送, 阻塞接收 | `Net_Client_Init()`, `Net_Client_SendFrame()`, `Net_Client_RecvCommand()` |
| **Modbus_Slave** | RS485 Modbus RTU 帧收发, 寄存器读写 | `Modbus_Init()`, `Modbus_Poll()` |
| **Transport_HAL** | 通信通道抽象层 | `Transport_Send()`, `Transport_Recv()` |
| **Alarm_Handler** | GPIO 驱动, 声光报警控制 | `Alarm_Init()`, `Alarm_Set()` |
| **Power_Manager** | V2 为三级功耗状态机; 雷达方案改为雷达事件驱动状态机 | `PowerMgr_Init()`, `PowerMgr_Tick()`, `PowerMgr_GetMode()` |
| **Auto_Exposure** | 亮度分析 + Laplacian 清晰度, SCCB 调参 | `AutoExp_Init()`, `AutoExp_Analyze()`, `AutoExp_Adjust()` |
| **Servo_Control** | 舵机 PWM 驱动 (扩展版, 基础版不启用) | `Servo_Init()`, `Servo_SetAngle()` |

## 3. 数据流设计

### 3.1 V2 并行数据流

```
OV5640
  │ DCMI + DMA (Snapshot)
  ▼
D2 SRAM 双缓冲 ──────────────────────────────────┐
  │ Buffer A (100KB)  │  Buffer B (100KB)          │
  │  ↕ 交替使用                                    │
  ▼                                                │
Task_Camera (生产者)                               │
  ├── scan_jpeg() / extract_gray()                 │
  ├── Motion_Detect / AutoExp (DETECT 模式)        │
  ├── PowerMgr_Tick()                              │
  └── osMessageQueuePut(s_frame_q, &fd) ──────┐   │
       立即切换到另一缓冲区采集下一帧          │   │
                                               ▼   │
                                    ┌──────────────┐│
                                    │ s_frame_q    ││
                                    │ (depth=2)    ││
                                    └──────┬───────┘│
                                           ▼        │
                                    Task_NetTx (消费者)
                                      ├── netbuf_ref() (PBUF_ROM 零拷贝)
                                      ├── netconn_send() × N 分片
                                      └── 释放缓冲区 → FREE

并行:
  Task_NetRx ── netconn_recv() (阻塞) ── 解析 IVCIS_Command_t ──┐
                                                                  │
  Task_RS485 ── UART IDLE + DMA ── Modbus RTU 解析 ──────────────┤
                  ├── 寄存器读 → 系统状态                          │
                  └── 寄存器写 ─────────────────────────────────┤
                                                                  ▼
                                                         ┌──────────────┐
                                                         │  统一执行层   │
                                                         │ Alarm_Set()  │
                                                         │ PowerMgr_ForceMode()
                                                         │ (扩展: Servo)│
                                                         └──────────────┘
  双控制源策略: Last-Write-Wins, 部署时可二选一或并存
  执行层 API 不关心指令来源 (UDP / RS485)
```

### 3.2 V2 双层运动检测方案

| 模式 | OV5640 输出 | 帧率 | 检测方法 | 上传 |
|------|------------|------|----------|------|
| FULL | JPEG 640×480 | ~10fps | JPEG 尺寸差 >15% | 以太网 UDP |
| LIGHT | JPEG 640×480 | ~2fps | JPEG 尺寸差 >15% | 以太网 UDP |
| DETECT | Gray 160×120 | ~1fps | 像素帧差法 | 不上传 |

状态转移:
- FULL → LIGHT: 30 秒无运动
- LIGHT → DETECT: 5 分钟无运动
- DETECT → FULL: 灰度帧差触发
- LIGHT 有运动 → FULL

> 雷达增强版中, 该视觉帧差链路不再作为主触发源。`Motion_Detect` 保留为雷达故障或无雷达 SKU 的 fallback, 主触发由雷达目标列表和抓拍区状态决定。

### 3.3 雷达增强版业务数据流 (V2.5 规划)

```
毫米波雷达模块
  │ UART/CAN/Ethernet, 输出目标列表
  ▼
Task_Radar
  ├── 解析 track_id / range / speed / azimuth / lane / confidence
  ├── 生成 RadarSummary_t: target_present / target_in_capture_zone / timeout
  └── 更新 Power_Manager / Fusion 状态
        │
        ├── 无目标: RADAR_GUARD, 相机低频健康抓拍或休眠
        ├── 目标接近: ARMED_STANDBY, 唤醒相机并准备曝光
        ├── 进入抓拍区: ACTIVE_CAPTURE, 触发 OV5640 JPEG 抓拍
        └── 雷达异常: FAULT_FALLBACK, 回退周期抓拍或视觉帧差

Task_Camera ── FrameDesc_t + RadarMeta ──▶ Task_NetTx
Task_NetTx  ── JPEG + RadarMeta over UDP ─▶ 云端/上位机融合识别
Task_RS485  ◀─ Modbus RTU ────────────────▶ PLC/HMI 读取雷达和融合状态
```

雷达增强版的设计边界:
- 雷达外设必须输出目标列表, 不把原始 ADC 或点云处理放到 `STM32H753`。
- `STM32H753` 负责轻量目标级融合、状态机、抓拍触发、协议转发和现场 I/O。
- 车牌、车型、复杂雷视融合和事件判断仍由云端/上位机完成。

### 3.4 图像质量闭环 (DETECT 模式 / 雷达 fallback)

```
灰度帧 → AutoExp_Analyze()
          ├── 平均亮度 (逐像素累加)
          ├── Laplacian 方差 (清晰度)
          └── 综合判定 → is_low_quality
                  │
                  ▼
          AutoExp_Adjust()
          └── SCCB 步进调整 OV5640 曝光等级
```

## 4. 内存布局

| 区域 | 地址范围 | 大小 | 用途 | 属性 |
|------|----------|------|------|------|
| **D1 AXI-SRAM** | 0x24000000 - 0x2407FFFF | 512KB | FreeRTOS Heap, 主栈, 应用变量 | Cacheable |
| **D2 SRAM** | 0x30000000 - 0x30047FFF | 288KB | 帧缓冲, ETH 描述符, LwIP Heap | Non-cacheable (MPU) |
| **D3 SRAM** | 0x38000000 - 0x3800FFFF | 64KB | 低功耗保留 | Non-cacheable |
| **Flash** | 0x08000000 - 0x081FFFFF | 2MB | 代码 + 只读数据 | - |

### D2 SRAM 详细分配 (V2)

```
0x30000000 ┌──────────────────────┐
           │ Frame Buffer A       │  100KB
0x30019000 ├──────────────────────┤
           │ Frame Buffer B       │  100KB
0x30032000 ├──────────────────────┤
           │ ETH DMA 描述符       │  ~1KB
           ├──────────────────────┤
           │ LwIP Heap (pbuf 池)  │  ~40KB
           ├──────────────────────┤
           │ 灰度帧缓冲/雷达元数据 │
           ├──────────────────────┤
           │ 预留                 │
0x30047FFF └──────────────────────┘
```

## 5. FreeRTOS 任务设计

| 任务名 | 优先级 | 栈大小 | 职责 |
|--------|--------|--------|------|
| **Task_Camera** | High (40) | 8KB | 采集 + 质量控制 + 功耗管理, 帧入队后立即采下一帧 |
| **Task_NetTx** | AboveNormal (32) | 8KB | 从帧队列取数据, Netconn UDP 零拷贝发送 |
| **Task_NetRx** | Normal (24) | 4KB | Netconn UDP 阻塞接收, 分发 Alarm/Servo 指令 |
| **Task_RS485** | Normal (24) | 2KB | Modbus RTU 从站: 帧解析, 寄存器读写 |
| **Task_Radar** | Normal (24) | 2KB | 读取雷达目标列表, 更新雷达摘要和抓拍触发状态 |

### 任务间通信

```
Task_Camera ──osMessageQueue──▶ Task_NetTx    (FrameDesc_t)
Task_NetRx  ──直接调用──▶ Alarm_Set() / PowerMgr_ForceMode()
Task_RS485  ──直接调用──▶ 读系统状态 / 触发拍照 / 设置参数
Task_Radar  ──共享摘要──▶ PowerMgr_TickRadar() / Vision_TriggerOnce()
```

> **设计决策**: Task_NetRx 从 Task_Net 中拆分出来, 因为 Netconn API 的 `netconn_recv()` 会阻塞, 不能和发送复用同一任务。RS485 是独立的物理通道, 独立任务处理。

## 6. 接口定义 (Header Files)

### 6.1 Vision_Pipeline.h

```c
typedef enum {
    VISION_MODE_JPEG,
    VISION_MODE_GRAY
} VisionMode_t;

int8_t   Vision_Init(void);
void     Vision_SetMode(VisionMode_t mode);
void     Vision_CaptureStart(void);
uint8_t  Vision_IsFrameReady(void);
uint8_t* Vision_GetFrameBuffer(void);
uint32_t Vision_GetFrameSize(void);
```

### 6.2 Net_Client.h (V2 Netconn)

```c
typedef enum { NET_IDLE, NET_READY, NET_SENDING, NET_ERROR } NetState_t;

int8_t     Net_Client_Init(void);
void       Net_Client_SendFrame(const FrameDesc_t *fd);
bool       Net_Client_RecvCommand(IVCIS_Command_t *cmd);
NetState_t Net_Client_GetState(void);
uint32_t   Net_Client_GetTxCount(void);
```

### 6.3 Transport_HAL.h (V2 新增)

```c
typedef enum { TRANSPORT_ETH, TRANSPORT_RS485, TRANSPORT_COUNT } TransportType_t;
typedef enum { TRANSPORT_OK, TRANSPORT_ERR_INIT, TRANSPORT_ERR_SEND,
               TRANSPORT_ERR_BUSY, TRANSPORT_ERR_TIMEOUT } TransportErr_t;

typedef struct {
    TransportErr_t (*init)(void);
    TransportErr_t (*send)(const uint8_t *buf, uint32_t len);
    TransportErr_t (*recv)(uint8_t *buf, uint32_t max_len,
                           uint32_t *out_len, uint32_t timeout_ms);
    void           (*deinit)(void);
    uint16_t       mtu;
    const char    *name;
} Transport_Ops_t;
```

### 6.4 Modbus_Slave.h (V2 新增)

```c
void    Modbus_Init(uint8_t slave_addr, uint32_t baudrate);
void    Modbus_Poll(void);   /* Task_RS485 主循环调用 */
```

### 6.5 Radar_Manager.h (V2.5 规划)

```c
typedef struct {
    uint32_t timestamp_ms;
    uint32_t track_id;
    int16_t  range_cm;
    int16_t  speed_cms;
    int16_t  azimuth_deg_x10;
    uint8_t  lane_id;
    uint8_t  confidence;
} RadarTarget_t;

typedef struct {
    bool     target_present;
    bool     target_in_capture_zone;
    bool     target_approaching;
    uint8_t  target_count;
    uint32_t last_update_ms;
} RadarSummary_t;

void Radar_Init(void);
void Radar_Poll(void);
bool Radar_GetSummary(RadarSummary_t *out);
```

### 6.6 其他模块 (不变)

```c
/* Alarm_Handler.h */
void Alarm_Init(void);
void Alarm_Set(bool active);

/* Motion_Detect.h */
void     Motion_Init(uint16_t width, uint16_t height);
bool     Motion_Detect(uint8_t *current_frame);
uint32_t Motion_GetDiff(void);

/* Power_Manager.h */
void        PowerMgr_Init(void);
PowerMode_t PowerMgr_Tick(uint32_t jpeg_size, bool gray_motion);
uint32_t    PowerMgr_GetDelay(void);
PowerMode_t PowerMgr_GetMode(void);

/* Auto_Exposure.h */
void           AutoExp_Init(void);
ImageQuality_t AutoExp_Analyze(uint8_t *frame);
void           AutoExp_Adjust(ImageQuality_t *quality);

/* Servo_Control.h */
void     Servo_Init(void);
void     Servo_SetAngle(uint16_t angle_deg);
```

## 7. 通信协议

> 详见 `09_CommunicationProtocol.md`

### 7.1 UDP 上报格式 (MCU → 云端)

```c
/* NetChunkHdr_t: 20 字节包头 */
typedef struct __attribute__((packed)) {
    uint32_t magic;       /* 0x49564349 ("IVCI") */
    uint32_t frame_id;
    uint16_t chunk_idx;
    uint16_t chunk_cnt;
    uint32_t total_size;
    uint8_t  flags;       /* bit0: low_quality, bit1: alarm_active */
    uint8_t  reserved[3];
} NetChunkHdr_t;
```

### 7.2 云端下行指令 (云端 → MCU)

```c
typedef struct __attribute__((packed)) {
    uint32_t magic;       /* 0x49564352 ("IVCR") */
    uint8_t  cmd_type;    /* 0x01=违规, 0x02=舵机 */
    uint8_t  reserved[3];
    union {
        struct { uint8_t is_violation; char plate[16]; } violation;
        struct { uint16_t angle_deg; } servo;
    } payload;
} IVCIS_Command_t;
```

### 7.3 网络配置

| 参数 | 默认值 |
|------|--------|
| 设备 IP | 192.168.1.10 |
| 目标 IP | 192.168.1.100 |
| 上行端口 | 8080 |
| 下行端口 | 8000 |

## 8. 引脚映射

### 8.1 ETH RMII (板载 LAN8742A)

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

### 8.2 DCMI (OV5640)

| 信号 | MCU Pin |
|------|---------|
| DCMI_HSYNC | PA4 |
| DCMI_VSYNC | PG9 |
| DCMI_PIXCLK | PA6 |
| DCMI_D0 | PC6 |
| DCMI_D1 | PC7 |
| DCMI_D2 | PE0 |
| DCMI_D3 | PE1 |
| DCMI_D4 | PE4 |
| DCMI_D5 | PB6 |
| DCMI_D6 | PE5 |
| DCMI_D7 | PE6 |

### 8.3 I2C1 SCCB (OV5640 配置)

| 信号 | MCU Pin |
|------|---------|
| I2C1_SCL | PB8 |
| I2C1_SDA | PB9 |

### 8.4 USART3 VCP (调试串口, 板载)

| 信号 | MCU Pin |
|------|---------|
| TX | PD8 |
| RX | PD9 |

### 8.5 RS485 (V2 新增, USART2)

| 信号 | MCU Pin | Zio 位置 | 连接 |
|------|---------|---------|------|
| USART2_TX | **PD5** | CN9 pin 6 (D53) | → MAX3485/SN65HVD75 DI |
| USART2_RX | **PD6** | CN9 pin 4 (D52) | ← MAX3485/SN65HVD75 RO |
| USART2_RTS (DE) | **PD4** | CN9 pin 8 (D54) | → DE & /RE (硬件自动控制) |

### 8.6 雷达外设接口 (V2.5 规划)

| 接口形态 | 建议 | 说明 |
|------|------|------|
| UART | 推荐 | 适合接收低速目标列表, 需要独立于 `USART2` 的串口 |
| CAN / FDCAN | 推荐 | 工业现场抗干扰更好, 适合目标状态和事件消息 |
| Ethernet | 可选 | 适合高端雷达模块, 但会增加网络拓扑复杂度 |
| 复用 RS485 | 不建议 | `USART2 + MAX3485` 已用于 PLC/HMI Modbus RTU, 复用会造成协议冲突 |

### 8.7 LED 指示

| LED | MCU Pin | 用途 |
|-----|---------|------|
| LD1 (Green) | PB0 | 系统状态 |
| LD2 (Yellow) | PE1 | 网络/运动指示 |
| LD3 (Red) | PB14 | 违规报警 |

### 8.8 舵机 PWM

| 信号 | MCU Pin | 定时器 |
|------|---------|--------|
| Servo PWM | PD15 | TIM4_CH4 (AF2) |

---

**文档版本**: v3.1
**更新日期**: 2026-04-26
**维护者**: IVCIS Team
**变更记录**:
- v3.1: 新增雷达增强版规划; `Motion_Detect` 降级为 fallback; 新增 `Radar_Manager/Task_Radar`; 功耗触发由视觉帧差转向雷达目标摘要; 补充雷达接口边界
- v3.0: 项目定位变更为工业智能相机; 新增 Transport 抽象层, Modbus_Slave 模块; 任务拓扑重构 (Task_Net 拆分为 Tx/Rx + Task_RS485); Netconn API 替代 Raw API; 双缓冲并行数据流; 新增 RS485 引脚预留
- v2.3: 三级功耗状态机; 双层检测; Vision_Pipeline 拆分
- v2.2: 非阻塞 RTOS 任务集成
