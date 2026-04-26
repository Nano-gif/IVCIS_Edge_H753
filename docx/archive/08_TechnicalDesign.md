# IVCIS 工业智能相机 - 技术设计文档

> **定位变更**: 本项目从"高速入口智能视觉终端"演进为通用型**工业智能相机**平台。
> 本文档记录当前实现的技术细节，并定义 V2 架构重构的目标设计。

---

## 1. 系统概述

### 1.1 硬件平台

| 资源 | 规格 | 备注 |
|------|------|------|
| MCU | STM32H753ZIT6 (Cortex-M7 @ 480MHz) | Nucleo-144 开发板 |
| D1 AXI-SRAM | 512KB (0x24000000) | FreeRTOS Heap, 主栈, 应用变量 |
| D2 SRAM | 288KB (0x30000000) | DCMI 帧缓冲, ETH 描述符, LwIP Heap |
| D3 SRAM | 64KB (0x38000000) | 低功耗保留区 |
| Flash | 2MB (0x08000000) | 代码 + 只读数据 |
| 摄像头 | OV5640 (DCMI 8-bit, SCCB/I2C1) | JPEG 640x480 / YUV422 160x120 |
| PHY | LAN8742A (RMII, 板载) | 100Mbps |
| 调试串口 | USART3 VCP (PD8/PD9) | 115200 8N1 |
| RS485 | USART2 (PD5/PD6/PD4-DE) | Modbus RTU, Zio CN9 |
| 雷达外设 | UART/CAN/Ethernet 模块 (规划) | 输出目标列表, 不接收原始 ADC/点云 |

### 1.2 软件栈

| 层级 | 组件 | 版本 |
|------|------|------|
| RTOS | FreeRTOS (CMSIS-RTOS v2) | CubeMX 生成 |
| 网络协议栈 | LwIP | v2.1.2 |
| HAL | STM32H7 HAL | CubeMX 生成 |
| 摄像头驱动 | 正点原子 OV5640 BSP | 自定义移植 |
| 调试输出 | nanoprintf + UART3 DMA | 自定义实现 |

---

## 2. V1 当前架构分析

### 2.1 任务模型

当前系统仅有两个 FreeRTOS 任务：

```
Task_Camera (osPriorityHigh, 8KB stack)
  │
  ├─ Vision_Init() + OV5640 配置
  ├─ PowerMgr_Init() / AutoExp_Init()
  │
  └─ for(;;) 主循环:
       ├─ Vision_CaptureStart()          ← 同步阻塞等 DMA 完成
       ├─ if DETECT: Motion_Detect() + AutoExp
       │  else: Net_Client_SendImage()   ← 同步阻塞发送 ★问题点
       ├─ PowerMgr_Tick()
       ├─ 模式切换 (JPEG ↔ Gray)
       └─ osDelay(PowerMgr_GetDelay())

Task_Net (osPriorityLow, 8KB stack)
  │
  ├─ Net_Client_Init()  (UDP bind + recv callback)
  │
  └─ for(;;):
       ├─ Net_Client_RecvCommand()  ← 轮询回调标志位
       ├─ 分发 Alarm / Servo 指令
       └─ osDelay(100)
```

### 2.2 关键问题

#### 问题 1: 采集与发送串行执行

`freertos.c:192` 中 `Net_Client_SendImage()` 在 Camera 任务内同步调用。发送一帧 50KB JPEG 需要 ~36 个 UDP 分片，每片间 `osDelay(1)`，总耗时 ~36ms。在此期间相机无法采集下一帧。

实际帧率 = 1 / (采集时间 + 发送时间 + 处理时间) ≈ 1 / (30ms + 36ms + 5ms) ≈ **14fps**

若采集与发送并行流水线化，理论帧率可提升至 ≈ 1 / max(30ms, 36ms) ≈ **27fps**。

#### 问题 2: LwIP Raw API 线程安全隐患

当前代码在 `Task_Camera` 中调用 `udp_sendto()` (Raw API)，而 `net_recv_cb` 回调运行在 `tcpip_thread` 上下文中。两者共享 `s_net` 结构体（尤其是 `s_net.state` 和 `s_net.new_cmd`），存在竞态条件。

Raw API 的设计约束：所有操作必须在 `tcpip_thread` 内执行，或者使用 `LOCK_TCPIP_CORE()` 加锁。当前代码均未满足。

#### 问题 3: 通信通道硬耦合

`Net_Client.c` 直接依赖 LwIP `udp_*` API，无法扩展到其他通信方式 (RS485 等)。应用层代码 (`freertos.c`) 直接调用 `Net_Client_SendImage()`，通信方式无法在运行时切换。

#### 问题 4: 单缓冲区

`Vision_Pipeline.c` 使用单一 `s_raw_buf[100KB]`。DCMI DMA 写入和网络发送读取指向同一块内存，虽然当前因为串行执行不会冲突，但在并行化后会产生数据撕裂。

### 2.3 数据流 (V1)

```
OV5640
  │ DCMI + DMA (Snapshot 模式)
  ▼
s_raw_buf[100KB] (D2 SRAM, 单缓冲)
  │
  ├── scan_jpeg() → s_frame_ptr / s_frame_len
  │   或 extract_gray() → s_gray_buf
  │
  ├── [JPEG 模式] Net_Client_SendImage()
  │     └── udp_sendto() × N 片 (PBUF_ROM, 零拷贝)
  │
  ├── [灰度模式] Motion_Detect()
  │     └── 帧差法 → bool
  │
  └── AutoExp_Analyze() + AutoExp_Adjust()
        └── SCCB 寄存器步进
```

---

## 3. V2 目标架构

### 3.1 设计原则

1. **采集与传输完全解耦**: 生产者-消费者模型，通过 RTOS 消息队列连接
2. **通信通道可插拔**: Transport 抽象层，以太网为主力，RS485 为控制面
3. **LwIP 使用 Netconn API**: 线程安全，阻塞语义适配 RTOS，支持后续 MQTT 扩展
4. **双缓冲真并行**: DMA 采集与网络发送物理隔离
5. **雷达目标级融合**: 雷达模块输出目标列表, H753 只做抓拍区判断、状态机和协议转发

### 3.2 任务拓扑

```
Task_Camera (High, 8KB)              Task_NetTx (AboveNormal, 8KB)
  │                                     │
  ├─ Vision_Init()                      ├─ Netconn UDP 初始化
  ├─ PowerMgr / AutoExp Init            ├─ Transport_ETH 注册
  └─ for(;;):                           └─ for(;;):
       ├─ Vision_CaptureStart(buf[A/B])      ├─ osMessageQueueGet(s_frame_q)
       ├─ AutoExp / 图像质量处理             ├─ Transport_Send(JPEG)
       ├─ PowerMgr_Tick()                    ├─ 发送 RadarMeta (V2.5)
       ├─ osMessageQueuePut(s_frame_q, &fd)  └─ 释放缓冲区占用标记
       └─ 切换到另一缓冲区

Task_NetRx (Normal, 4KB)             Task_RS485 (Normal, 2KB)
  │                                     │
  ├─ Netconn UDP bind + listen          ├─ UART DMA 接收初始化
  └─ for(;;):                           └─ for(;;):
       ├─ netconn_recv()                    ├─ Modbus RTU 帧解析
       ├─ 解析 IVCIS_Command_t              ├─ 寄存器读写处理
       └─ 分发到统一执行 API ──┐             └─ 分发到统一执行 API ──┐
                               │                                     │
Task_Radar (Normal, 2KB)      │                                     │
  │                            │                                     │
  ├─ 解析目标列表               │                                     │
  ├─ 更新 RadarSummary          │                                     │
  └─ 触发抓拍/功耗状态 ─────────┘                                     │
                               ▼                                     ▼
                          ┌──────────────────────────────────────────────┐
                          │ 统一执行层 (接口无关, Last-Write-Wins)         │
                          │  Alarm_Set()  PowerMgr_ForceMode()          │
                          │  Vision_TriggerOnce()                       │
                          │  (扩展版: Servo_SetAngle)                   │
                          └──────────────────────────────────────────────┘
```

> **双控制源设计**: Task_NetRx (云端 UDP 指令) 和 Task_RS485 (RS485 Modbus 寄存器写入) 均可触发相同的执行动作。两条控制路径在部署时可**二选一或同时启用**:
> - **纯云端模式**: 仅使用以太网，无 RS485 收发器，Task_RS485 不创建
> - **纯本地模式**: PLC/HMI 通过 RS485 直接控制，无需云端
> - **混合模式**: 两者并存，Last-Write-Wins 语义，无仲裁锁
>
> 执行层 API (`Alarm_Set` 等) 是原子操作 (GPIO 寄存器写入)，天然线程安全，无需额外同步。舵机控制 (`Servo_SetAngle`) 仅在扩展版中启用。
>
> **雷达增强版**: 新增 `Task_Radar` 后, 雷达目标列表成为主触发源。视觉帧差不再用于常规唤醒, 仅在雷达故障、无雷达 SKU 或调试模式中作为 fallback。

### 3.3 帧缓冲管理

```
D2 SRAM 布局 (288KB 可用):

0x30000000 ┌──────────────────────┐
           │ Frame Buffer A       │  100KB  ← DCMI DMA 目标 (交替)
0x30019000 ├──────────────────────┤
           │ Frame Buffer B       │  100KB  ← DCMI DMA 目标 (交替)
0x30032000 ├──────────────────────┤
           │ ETH DMA 描述符       │  ~1KB
           ├──────────────────────┤
           │ LwIP Heap (pbuf池)   │  ~40KB
           ├──────────────────────┤
           │ 灰度帧 (19.2KB)     │  用于帧差检测
           ├──────────────────────┤
           │ 预留                 │
0x30047FFF └──────────────────────┘
```

**缓冲区状态机**:

```
每个缓冲区有三种状态:
  FREE     → Camera 可写入 (DMA 目标)
  QUEUED   → 已入队，等待 Net 任务消费
  SENDING  → Net 任务正在发送，不可覆写

状态流转:
  Camera: FREE → [DMA采集] → QUEUED → osMessageQueuePut()
  NetTx:  osMessageQueueGet() → SENDING → [发送完毕] → FREE

背压策略:
  if (队列满 && 无 FREE 缓冲区):
    丢弃当前帧, DBG_WARN("[Camera] Frame dropped: backpressure")
    Camera 继续下一轮采集，永不阻塞
```

### 3.4 Transport 抽象层

```c
/* Transport_HAL.h */

typedef enum {
    TRANSPORT_ETH = 0,   /* 以太网 (Netconn UDP) */
    TRANSPORT_RS485,     /* RS485 (Modbus RTU)   */
    TRANSPORT_COUNT
} TransportType_t;

typedef enum {
    TRANSPORT_OK = 0,
    TRANSPORT_ERR_INIT,
    TRANSPORT_ERR_SEND,
    TRANSPORT_ERR_BUSY,
    TRANSPORT_ERR_TIMEOUT
} TransportErr_t;

/* 传输通道操作虚表 */
typedef struct {
    TransportErr_t (*init)(void);
    TransportErr_t (*send)(const uint8_t *buf, uint32_t len);
    TransportErr_t (*recv)(uint8_t *buf, uint32_t max_len,
                           uint32_t *out_len, uint32_t timeout_ms);
    void           (*deinit)(void);
    uint16_t       mtu;        /* 该通道最大传输单元 */
    const char    *name;       /* 用于日志标识 */
} Transport_Ops_t;
```

**通道职责分工**:

| 通道 | 数据面 (图传) | 控制面 (指令) | 配置面 (参数) |
|------|:---:|:---:|:---:|
| **以太网** | **主力** | 支持 | 支持 |
| **RS485** | 不支持 | **主力** | 支持 |

以太网承载图传 + 云平台信令 (MQTT)；RS485 承载工业现场 Modbus RTU 指令交互。两者独立运行，互不影响。

### 3.5 LwIP Netconn 迁移方案

**为什么选 Netconn 而不是 Socket**:

| 维度 | Netconn API | Socket API |
|------|-------------|------------|
| 线程安全 | 内建，通过 mbox 与 tcpip_thread 通信 | 内建，底层封装 Netconn |
| RAM 开销 | 较低 (~200B/连接) | 较高 (~500B/连接, 多一层 fd 表) |
| 零拷贝 | `netbuf_ref()` + PBUF_ROM | 不支持，必须 `send()` 拷贝 |
| MQTT 兼容 | lwip 内置 mqtt client 基于 Raw API，但第三方库 (如 coreMQTT) 可适配 Netconn | 大多数 MQTT 库原生支持 |
| POSIX 兼容 | 否 | 是 |

**结论**: Netconn 在本项目中是最优选择 — 保留零拷贝能力，线程安全，开销最低。

**Netconn UDP 发送核心流程**:

```c
/* 初始化 (Task_NetTx 启动时) */
struct netconn *s_conn = netconn_new(NETCONN_UDP);
netconn_bind(s_conn, IP_ADDR_ANY, UDP_LOCAL_PORT);
netconn_connect(s_conn, &dest_addr, UDP_REMOTE_PORT);

/* 发送单帧 (零拷贝分片) */
void NetTx_SendFrame(const FrameDesc_t *fd) {
    uint32_t offset = 0;
    while (offset < fd->data_len) {
        uint32_t chunk = MIN(fd->data_len - offset, NET_MAX_UDP_PAYLOAD);
        struct netbuf *nb = netbuf_new();
        netbuf_ref(nb, fd->p_data + offset, chunk);  /* PBUF_ROM 零拷贝 */
        netconn_send(s_conn, nb);
        netbuf_delete(nb);
        offset += chunk;
    }
}
```

**Netconn UDP 接收 (替代回调轮询)**:

```c
/* Task_NetRx: 阻塞式接收，不再需要 osDelay 轮询 */
void StartNetRxTask(void *arg) {
    struct netconn *conn = netconn_new(NETCONN_UDP);
    netconn_bind(conn, IP_ADDR_ANY, UDP_LOCAL_PORT);

    for (;;) {
        struct netbuf *nb;
        if (netconn_recv(conn, &nb) == ERR_OK) {
            void *data;
            uint16_t len;
            netbuf_data(nb, &data, &len);
            /* 解析 IVCIS_Command_t ... */
            netbuf_delete(nb);
        }
    }
}
```

### 3.6 中断与 DMA 策略

| 外设 | DMA Stream | 中断 | 说明 |
|------|-----------|------|------|
| DCMI | DMA2 (CubeMX 分配) | `DMA2_StreamX_IRQn` + `DCMI_IRQn` | Snapshot 模式，帧完成中断置标志 |
| ETH TX | 内置 DMA | `ETH_IRQn` | LwIP 驱动管理 |
| ETH RX | 内置 DMA | `ETH_IRQn` | LwIP 驱动管理 |
| USART3 TX | DMA1 (可选) | - | nanoprintf 输出，可 polling 或 DMA |
| RS485 UART RX | DMA1 Stream | `USARTx_IRQn` (IDLE) | IDLE 中断 + DMA 接收不定长 Modbus 帧 |

**DCMI 帧完成中断**:

```c
/* 当前实现: 轮询标志位 */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi) {
    g_ov5640_frame_cplt = 1;  /* vision_capture.c 中 while 轮询此标志 */
}

/* V2 目标: 信号量唤醒，消除轮询 */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi) {
    osSemaphoreRelease(s_frame_sem);  /* 唤醒 Task_Camera */
}
```

### 3.7 时钟树与外设分配

```
SYSCLK = 480 MHz (HSE 8MHz → PLL1)
  ├── AHB  = 240 MHz
  ├── APB1 = 120 MHz (TIM2/3/4/5/6/7, USART2/3, I2C1, SPI2/3)
  │     └── TIM4: 舵机 PWM (50Hz)
  │     └── USART3: 调试 VCP (115200)
  │     └── I2C1: OV5640 SCCB (PB8/PB9)
  │     └── USARTx: RS485 (待分配)
  ├── APB2 = 120 MHz (USART1, SPI1, TIM1/8)
  ├── DCMI Clock = AHB (240 MHz)
  └── ETH  = AHB (240 MHz), RMII REF_CLK = 50MHz (外部)
```

### 3.8 雷达增强版状态机 (V2.5 规划)

接入雷达外设后, `Power_Manager` 不再以 `jpeg_size_changed()` 和 `gray_motion` 作为主判断依据, 而是基于雷达摘要状态驱动相机工作模式。

| 状态 | 触发条件 | 相机行为 | 上传策略 |
|---|---|---|---|
| `RADAR_GUARD` | 雷达无目标, 或目标远离抓拍区 | 相机休眠/低频健康抓拍 | 不上传或低频上传健康帧 |
| `ARMED_STANDBY` | 雷达发现目标接近 | 唤醒相机, 准备曝光和缓存 | 可上传低频预览/状态 |
| `ACTIVE_CAPTURE` | 目标进入抓拍区 | JPEG 抓拍, 绑定雷达目标元数据 | 上传 JPEG + RadarMeta |
| `FAULT_FALLBACK` | 雷达超时、通信错误、目标列表异常 | 回退周期抓拍或视觉帧差 | 上传低频 JPEG, 标记 radar_fault |

推荐接口从旧版:

```c
PowerMode_t PowerMgr_Tick(uint32_t jpeg_size, bool gray_motion);
```

演进为:

```c
typedef enum {
    PWR_RADAR_GUARD,
    PWR_ARMED_STANDBY,
    PWR_ACTIVE_CAPTURE,
    PWR_FAULT_FALLBACK
} PowerMode_t;

PowerMode_t PowerMgr_TickRadar(const RadarSummary_t *radar,
                               const ImageQuality_t *quality);
```

设计原因:
- 帧差法计算量小, 但雨雾、眩光、低照度下可靠性不足。
- 雷达目标存在、距离、速度、方向更适合作为全天候触发源。
- H753 只做目标级状态机, 不做雷达原始 ADC、点云聚类或复杂多目标融合。
---

## 4. FreeRTOS 详细设计

### 4.1 内核配置 (FreeRTOSConfig.h)

| 配置项 | 值 | 说明 |
|--------|-----|------|
| `configUSE_PREEMPTION` | 1 | 抢占式调度，高优先级任务立即抢占低优先级 |
| `configCPU_CLOCK_HZ` | SystemCoreClock (480MHz) | 内核时钟 |
| `configTICK_RATE_HZ` | 1000 | 系统节拍 1ms，`osDelay(1)` = 1ms |
| `configMAX_PRIORITIES` | 56 | CMSIS-RTOS v2 映射: osPriorityHigh=40, Normal=24, Low=8 |
| `configMINIMAL_STACK_SIZE` | 128 words (512B) | Idle 任务最小栈 |
| `configTOTAL_HEAP_SIZE` | 65536 (64KB) | heap_4 管理，位于 D1 AXI-SRAM |
| `configUSE_MUTEXES` | 1 | 启用互斥量 (含优先级继承) |
| `configUSE_RECURSIVE_MUTEXES` | 1 | 启用递归互斥量 |
| `configUSE_COUNTING_SEMAPHORES` | 1 | 启用计数信号量 |
| `configUSE_TIMERS` | 1 | 启用软件定时器 (Timer Task prio=2, queue=10) |
| `configSUPPORT_STATIC_ALLOCATION` | 1 | 允许静态分配任务/队列 |
| `configSUPPORT_DYNAMIC_ALLOCATION` | 1 | 允许动态分配 (heap_4) |
| `configENABLE_FPU` | 0 | FPU 上下文切换未由 FreeRTOS 管理 (由 NVIC lazy stacking 处理) |
| `configENABLE_MPU` | 0 | MPU 由 HAL 初始化阶段配置，非 FreeRTOS 管理 |
| `USE_FreeRTOS_HEAP_4` | 定义 | 使用 heap_4 (首次适配 + 合并空闲块) |

**中断优先级边界**:

```
configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5
configLIBRARY_LOWEST_INTERRUPT_PRIORITY = 15

中断优先级 0-4: FreeRTOS 不可管理区 (不能调用 xxxFromISR)
中断优先级 5-15: 可安全调用 FreeRTOS API (xSemaphoreGiveFromISR 等)
```

> DCMI 和 DMA 中断必须配置在 5-15 范围内，否则 `xxxFromISR` 调用会导致硬件断言失败。

### 4.2 CMSIS-RTOS v2 封装层

本项目使用 CMSIS-RTOS v2 API 封装 FreeRTOS 原生 API，提供可移植性。常用映射关系：

| CMSIS-RTOS v2 | FreeRTOS 原生 | 本项目使用场景 |
|----------------|--------------|---------------|
| `osThreadNew()` | `xTaskCreate()` | 创建 Task_Camera, Task_Net 等 |
| `osDelay()` | `vTaskDelay()` | 帧间隔等待、模式切换稳定延时 |
| `osMessageQueueNew()` | `xQueueCreate()` | V2: 帧队列 (FrameDesc_t) |
| `osMessageQueuePut()` | `xQueueSend()` | V2: Camera 入队帧描述符 |
| `osMessageQueueGet()` | `xQueueReceive()` | V2: NetTx 取出帧描述符 |
| `osSemaphoreNew()` | `xSemaphoreCreateBinary()` | V2: DCMI 帧完成信号量 |
| `osSemaphoreRelease()` | `xSemaphoreGive()` | V2: ISR 中通知帧就绪 |
| `osSemaphoreAcquire()` | `xSemaphoreTake()` | V2: Camera 等待帧就绪 |
| `osMutexNew()` | `xSemaphoreCreateMutex()` | 共享状态保护 (如 Net 状态) |
| `vTaskSuspend(NULL)` | - | 测试模式执行完毕后挂起自身 |

### 4.3 V1 当前同步机制

V1 的任务同步非常简单（也是其主要缺陷来源）：

```
┌───────────────────────────────────────────────────────┐
│ V1 同步机制 (当前代码)                                  │
│                                                        │
│ 1. DCMI 帧完成通知:                                    │
│    ISR: g_ov5640_frame_cplt = 1  (volatile 标志位)     │
│    Task_Camera: while(g_ov5640_frame_cplt == 0)        │
│                   osDelay(1);     ← 1ms 轮询          │
│                                                        │
│ 2. Task_Camera → Task_Net 数据传递:                    │
│    ⚠ 无！Net_Client_SendImage() 在 Camera 任务        │
│    内直接调用，完全串行                                 │
│                                                        │
│ 3. 云端指令接收:                                       │
│    net_recv_cb() 在 tcpip_thread 中设置 s_net.new_cmd  │
│    Task_Net: if(s_net.new_cmd) ... osDelay(100) 轮询   │
│    ⚠ new_cmd 无互斥保护 (volatile bool)               │
│                                                        │
│ 4. 共享状态:                                           │
│    s_net.state: Task_Camera 写, Task_Net 读            │
│    ⚠ 无互斥保护，存在竞态条件                          │
└───────────────────────────────────────────────────────┘
```

**问题分析**:

| 问题 | 位置 | 影响 | 严重程度 |
|------|------|------|---------|
| volatile 轮询浪费 CPU | `vision_capture.c:33-39` | DMA 传输期间 Camera 任务每 1ms 唤醒一次检查标志，浪费 CPU 时间片 | 低 |
| 采集-发送串行 | `freertos.c:192` | 帧率被限制在 ~14fps | 高 |
| Raw API 跨线程 | `Net_Client.c` 在 Camera 中调用 `udp_sendto` | LwIP Raw API 不保证线程安全 | 高 |
| 无互斥的共享变量 | `s_net.state`, `s_net.new_cmd` | 理论上存在撕裂读写（实际因为都是原子宽度变量，风险较低） | 低 |

### 4.4 V2 目标同步机制

```
┌───────────────────────────────────────────────────────────┐
│ V2 同步机制 (目标设计)                                     │
│                                                            │
│ ① DCMI 帧完成: 二值信号量 (替代 volatile 轮询)            │
│                                                            │
│    DCMI ISR                    Task_Camera                 │
│    ─────────                   ───────────                 │
│    osSemaphoreRelease          osSemaphoreAcquire          │
│    (s_frame_sem)               (s_frame_sem, timeout)      │
│         │                            │                     │
│         └──── ISR→Task 唤醒 ────────┘                     │
│              零轮询, CPU 释放给其他任务                     │
│                                                            │
│ ② 帧传递: 消息队列 (生产者-消费者)                         │
│                                                            │
│    Task_Camera                 Task_NetTx                  │
│    ───────────                 ──────────                  │
│    osMessageQueuePut           osMessageQueueGet           │
│    (s_frame_q, &fd, 0)        (s_frame_q, &fd, osWait)    │
│         │                            │                     │
│         └──── 异步解耦 ────────────┘                      │
│    timeout=0: 队列满时立即返回,                            │
│    Camera 丢帧不阻塞                                       │
│                                                            │
│ ③ 云端指令: Netconn 内建 mbox (替代回调+轮询)             │
│                                                            │
│    tcpip_thread                Task_NetRx                  │
│    ────────────                ──────────                  │
│    LwIP 内部投递到             netconn_recv()              │
│    conn->recvmbox             (阻塞等待, 零轮询)           │
│                                                            │
│ ④ 缓冲区状态: 原子操作 或 互斥量                           │
│                                                            │
│    buf_state[2]: 枚举值 (FREE/QUEUED/SENDING)              │
│    Camera 写 FREE→QUEUED, NetTx 写 QUEUED→SENDING→FREE   │
│    单写者模型 — 每个状态转换只有一个任务执行                 │
│    无需互斥量 (天然互斥)                                    │
│                                                            │
│ ⑤ Modbus 寄存器: 互斥量保护共享状态读取                    │
│                                                            │
│    Task_RS485 读取 PowerMgr_GetMode() 等                   │
│    这些函数读取的 static 变量由 Task_Camera 写入            │
│    对于 32-bit 对齐的变量读取在 Cortex-M7 上是原子的       │
│    无需额外互斥 (单写者-多读者, 变量为原子宽度)            │
└───────────────────────────────────────────────────────────┘
```

### 4.5 任务优先级与调度策略

```
优先级  任务              行为            理由
─────────────────────────────────────────────────────────
 40     Task_Camera       周期性         最高优先级: DMA 帧完成后必须
        (High)            (信号量唤醒     立即处理, 否则丢帧或 DMA 覆写
                           + 周期延时)

 32     Task_NetTx        事件驱动       次高: 帧队列有数据就发送,
        (AboveNormal)     (队列阻塞)     避免队列堆积导致 Camera 丢帧

 24     Task_NetRx        事件驱动       中等: 云端指令不紧急,
        (Normal)          (netconn 阻塞) 但需要及时响应 (100ms 级)

 24     Task_RS485        事件驱动       中等: Modbus 请求需在
        (Normal)          (IDLE 中断)     3.5 字符时间内响应

 24     Task_Radar        事件驱动       中等: 雷达目标列表解析,
        (Normal)          (UART/CAN)      更新抓拍触发摘要

 24     tcpip_thread      事件驱动       LwIP 内部线程, 处理协议栈
        (lwipopts.h)      (mbox)         收发事件

  2     TimerTask         定时器回调     FreeRTOS 软件定时器服务

  0     Idle Task         空闲           CPU 无事可做时运行
─────────────────────────────────────────────────────────
```

> **抢占规则**: Task_Camera 在信号量就绪时立即抢占所有低优先级任务。Task_NetTx 在队列有数据时抢占 NetRx/RS485/Radar。同优先级任务 (NetRx, RS485, Radar, tcpip_thread) 之间时间片轮转。

### 4.6 FreeRTOS 对象清单

| 对象类型 | 名称 | 深度/计数 | 创建方式 | 用途 |
|----------|------|----------|----------|------|
| **MessageQueue** | `s_frame_q` | depth=2, item=`FrameDesc_t` | 动态 | Camera→NetTx 帧传递 |
| **BinarySemaphore** | `s_frame_sem` | - | 动态 | DCMI ISR→Camera 帧完成通知 |
| **Thread** | Task_Camera | stack=8KB | CubeMX 静态属性 + `osThreadNew` | 采集主循环 |
| **Thread** | Task_NetTx | stack=8KB | `osThreadNew` | 帧发送 |
| **Thread** | Task_NetRx | stack=4KB | `osThreadNew` | 指令接收 |
| **Thread** | Task_RS485 | stack=2KB | `osThreadNew` | Modbus 从站 |
| **Thread** | Task_Radar | stack=2KB | `osThreadNew` | 雷达目标列表解析和抓拍区判断 |

### 4.7 Heap 内存预算

当前 `configTOTAL_HEAP_SIZE = 64KB` (heap_4, D1 AXI-SRAM):

| 消费者 | 估算 | 说明 |
|--------|------|------|
| Task_Camera TCB + Stack | ~8.2KB | TCB ~340B + 8KB stack |
| Task_NetTx TCB + Stack | ~8.2KB | |
| Task_NetRx TCB + Stack | ~4.2KB | |
| Task_RS485 TCB + Stack | ~2.2KB | |
| Task_Radar TCB + Stack | ~2.2KB | 雷达目标列表解析, 不含大点云缓存 |
| tcpip_thread TCB + Stack | ~4.2KB | lwipopts: TCPIP_THREAD_STACKSIZE=1024 words |
| Timer Task TCB + Stack | ~1.3KB | configTIMER_TASK_STACK_DEPTH=256 words |
| Idle Task TCB + Stack | ~0.8KB | configMINIMAL_STACK_SIZE=128 words |
| s_frame_q | ~64B | 2 × sizeof(FrameDesc_t) + 队列控制块 |
| s_frame_sem | ~80B | 信号量控制块 |
| **合计** | **~29KB** | 64KB 预算充裕, 剩余 ~35KB |

> **注意**: 帧缓冲区 (200KB) 和 LwIP Heap 在 D2 SRAM, 不占用 FreeRTOS Heap。

### 4.8 关键时序分析

```
时间轴 (ms)     0    30    36    60    66    90
                │     │     │     │     │     │
V1 串行:        ├─采集─┤─发送──┤─采集─┤─发送──┤─采集─┤
                        ↑ Camera 阻塞
                帧周期 = 采集(30ms) + 发送(36ms) = 66ms → ~15fps

V2 并行:        ├─ 采集A ─┤─ 采集B ─┤─ 采集A ─┤
                     ├─── 发送A ──┤─── 发送B ──┤
                        ↑ 流水线重叠
                帧周期 = max(采集, 发送) = 36ms → ~27fps

时序细节:
  [0ms]   Camera: 信号量等待 → DCMI DMA 启动 (Buffer A)
  [30ms]  DCMI ISR: 帧完成 → osSemaphoreRelease(s_frame_sem)
          Camera 被唤醒 (抢占 NetTx)
  [30ms]  Camera: scan_jpeg() → osMessageQueuePut → 切换 Buffer B → DMA 启动
  [31ms]  Camera: osDelay 或 osSemaphoreAcquire (等下一帧)
          NetTx 被调度: osMessageQueueGet → 开始发送 Buffer A
  [60ms]  Camera: Buffer B 完成 → 入队 → 切换 Buffer A
          NetTx: Buffer A 发送完毕 → 标记 FREE → 取 Buffer B 开始发送
```

### 4.9 LwIP 与 FreeRTOS 的集成

LwIP 在 FreeRTOS 模式下有自己的 `tcpip_thread`，通过 `sys_arch.c` 适配层与 RTOS 对接:

```
lwipopts.h 关键配置:
  WITH_RTOS                  = 1      (启用 OS 适配)
  SYS_LIGHTWEIGHT_PROT       = 1      (启用临界区保护)
  TCPIP_THREAD_STACKSIZE     = 1024   (4KB stack)
  TCPIP_THREAD_PRIO          = 24     (osPriorityNormal)
  TCPIP_MBOX_SIZE            = 6      (tcpip_thread 邮箱深度)
  DEFAULT_UDP_RECVMBOX_SIZE  = 6      (UDP 接收邮箱深度)
  LWIP_NETCONN               = 1      (默认启用, opt.h 中定义)
  LWIP_SOCKET                = 1      (默认启用, 本项目不使用)
  LWIP_RAM_HEAP_POINTER      = 0x30044000  (LwIP Heap 位于 D2 SRAM)
  MEM_SIZE                   = 16360  (~16KB LwIP 内部堆)
  CHECKSUM_BY_HARDWARE       = 1      (STM32H7 ETH 硬件校验和)
```

**线程模型**:

```
                  ┌──────────────┐
                  │ tcpip_thread │ ← LwIP 内部创建
                  │  (prio=24)   │
                  │              │
                  │ 处理:        │
                  │ - ETH RX     │
                  │ - 定时器     │
                  │ - API 请求   │
                  └──────┬───────┘
                         │
              ┌──────────┼──────────┐
              │          │          │
         ┌────┴────┐ ┌──┴───┐ ┌───┴────┐
         │Netconn  │ │ARP   │ │DHCP    │
         │API 请求 │ │处理  │ │(未启用) │
         └────┬────┘ └──────┘ └────────┘
              │
    ┌─────────┼─────────┐
    │         │         │
Task_NetTx Task_NetRx  (其他)
netconn_    netconn_
send()      recv()
    │         │
    └─ 内部通过 mbox 投递到 tcpip_thread 执行 ─┘
```

> **关键**: Netconn API 的所有底层操作都通过邮箱投递给 `tcpip_thread` 执行，调用者任务在投递后阻塞等待结果。这保证了 LwIP 核心代码始终在单线程中运行，无需额外加锁。

### 4.10 ISR 安全规则

| 中断源 | 优先级 | 可调用 FreeRTOS API | 说明 |
|--------|--------|:---:|------|
| DCMI Frame Complete | 5-15 (需确认) | 是 | `osSemaphoreRelease` (内部调用 `xSemaphoreGiveFromISR`) |
| DMA2 Stream (DCMI) | 5-15 | 是 | HAL 内部使用 |
| ETH | 5-15 | 是 | LwIP `ethernetif` 驱动 |
| USART IDLE (RS485) | 5-15 | 是 | 通知 Task_RS485 帧接收完成 |
| SysTick | 15 (最低) | - | FreeRTOS 内核节拍 (1ms) |
| HardFault/NMI | 0 (最高) | 否 | 不可屏蔽 |

**ISR 编码规范**:
1. ISR 中只做: 置标志 / 释放信号量 / 发送队列 (xxxFromISR 变体)
2. 禁止在 ISR 中: `osDelay()`, `osMutexAcquire()`, 阻塞式 API
3. `portYIELD_FROM_ISR(xHigherPriorityTaskWoken)`: 当 ISR 唤醒了更高优先级任务时, 确保退出 ISR 后立即上下文切换

---

## 5. RS485 / Modbus RTU 集成设计

### 5.1 硬件接口

#### 5.1.1 UART 选型分析

NUCLEO-H753ZI 板载 USART/UART 资源使用情况:

| 外设 | 引脚 | 当前用途 | 可用于 RS485 |
|------|------|---------|:---:|
| USART3 | PD8 (TX), PD9 (RX) | VCP 调试串口 (ST-LINK) | ✗ |
| LPUART1 | PB6 (TX), PB7 (RX) | PB6 已被 DCMI_D5 占用 | ✗ |
| **USART2** | **PD5 (TX), PD6 (RX)** | **空闲** | **✓ 推荐** |
| UART4 | PA0 (TX), PA1 (RX) | PA1 已被 ETH RMII_REF_CLK 占用 | ✗ |
| UART7 | PE8 (TX), PE7 (RX) | 空闲, 但无硬件 RTS | △ 备选 |

**结论: 选用 USART2 (PD4/PD5/PD6)**

理由:
1. PD5, PD6 均空闲, 无引脚冲突
2. PD4 (USART2_RTS) 可配置为**硬件 DE 控制** (STM32H7 UART 支持 `RS485_DE` 模式)
3. 三个引脚均暴露在 Zio CN9 连接器上 (D53/D52/D54), 方便外接 RS485 收发器模块
4. 同一 GPIO 端口 (GPIOD), 时钟使能已开启 (DCMI 和 USART3 均使用了 GPIOD)

#### 5.1.2 引脚分配

| 信号 | MCU Pin | Zio 位置 | 连接 MAX485 | AF 功能 |
|------|---------|---------|------------|---------|
| USART2_TX | **PD5** | CN9 pin 6 (D53) | → DI (数据输入) | AF7 |
| USART2_RX | **PD6** | CN9 pin 4 (D52) | ← RO (数据输出) | AF7 |
| USART2_RTS (DE) | **PD4** | CN9 pin 8 (D54) | → DE & /RE (方向控制) | AF7 |

#### 5.1.3 硬件连接示意

```
                    NUCLEO-H753ZI                    RS485 收发器
                 ┌──────────────┐                ┌──────────────┐
                 │         PD5  │───── TX ──────▶│ DI           │
                 │  USART2 PD6  │◀──── RX ──────│ RO           │──── A/B 差分总线
                 │         PD4  │───── DE ──────▶│ DE ┰ /RE     │
                 │              │                │              │
                 │         GND  │────────────────│ GND          │
                 │         3V3  │────────────────│ VCC          │
                 └──────────────┘                └──────────────┘

  注: DE 和 /RE 短接, 由 PD4 统一控制 (高=发送, 低=接收)
  STM32H7 硬件 DE 模式下, USART 自动在发送时拉高 DE, 完成后拉低
```

#### 5.1.4 收发器选型建议

| 型号 | 电压 | 特点 | 推荐度 |
|------|------|------|:---:|
| MAX485 | 5V | 经典, 需电平转换 | △ |
| MAX3485 | 3.3V | 3.3V 原生, 引脚兼容 MAX485 | ★★★ |
| SP3485 | 3.3V | 低功耗, 10Mbps | ★★ |
| SN65HVD75 | 3.3V | 工业级, ESD 保护, 热关断 | ★★★ |

**推荐 MAX3485 或 SN65HVD75** — 3.3V 直连, 无需电平转换, 降低BOM复杂度。

**电气参数**: 半双工差分信号, 9600-115200 bps, 最大 32 节点 / 1200m

#### 5.1.5 CubeMX 配置要点

```
USART2:
  Mode: Asynchronous
  Hardware Flow Control: RS485 DE (使用 RTS pin)
  Baud Rate: 9600 (默认, 可通过 Modbus 寄存器配置)
  Word Length: 8 Bits
  Stop Bits: 1
  Parity: None

  Advanced:
    DE polarity: High
    DE assertion time: 8 (1/16 bit time @ 9600 = ~6.5μs)
    DE deassertion time: 8

  DMA:
    USART2_RX: DMA1 Stream x, Peripheral to Memory, Normal, Byte
    (发送使用中断模式即可, Modbus 帧短)

  NVIC:
    USART2 global interrupt: Enable (IDLE 中断检测帧结束)
```

### 5.2 Modbus RTU 寄存器设计 (从站)

设备作为 Modbus RTU **从站**，响应主站 (PLC/HMI) 的寄存器读写请求。

完整的寄存器表详见 `docx/09_CommunicationProtocol.md` §3.3-3.4，此处仅列出摘要:

| 区域 | 地址范围 | 读写 | 说明 |
|------|---------|------|------|
| 设备信息 | 0x0000-0x000F | R | 型号, 版本, 运行时长 |
| 状态监控 | 0x0010-0x001F | R | 功耗模式, 网络状态, 帧计数, 画质 |
| 控制指令 | 0x0020-0x002F | R/W | 触发拍照, 强制模式, 舵机, 报警, 复位 |
| 参数配置 | 0x0030-0x003F | R/W | 从站地址, 波特率, 目标IP/端口, 帧率, 超时 |
| 离散输入 | FC02 0x0000-0x0005 | R | 运动, 摄像头, 网络, 质量, 报警, 舵机状态 |

### 5.3 软件架构

#### 5.3.1 模块划分

```
APP/src/
  ├── Modbus_Slave.c        Modbus RTU 协议引擎 (CRC, 帧解析, 异常响应)
  ├── Modbus_RegMap.c       寄存器表回调 (读写映射到系统状态)
  └── Transport_RS485.c     Transport_HAL RS485 后端 (UART 初始化/收发)

APP/Inc/
  ├── Modbus_Slave.h        公共 API: Modbus_Init, Modbus_Poll
  ├── Modbus_RegMap.h       寄存器读写回调接口
  └── Transport_RS485.h     Transport_Ops_t RS485 实现
```

#### 5.3.2 Task_RS485 设计

```c
/* 优先级: osPriorityNormal, 栈: 2KB */
static void StartRS485Task(void *argument) {
    Modbus_Init(MODBUS_SLAVE_ADDR, &huart2);
    for (;;) {
        /* 阻塞等待 UART IDLE 中断 (信号量) */
        Modbus_Poll();  /* 内部: 解析请求 → 查寄存器表 → 构造响应 → 发送 */
    }
}
```

#### 5.3.3 接收机制: UART IDLE + DMA

```
                     DMA 连续接收
  UART RX ──────▶ ┌──────────────────┐
                  │  环形 DMA 缓冲区  │ (256B)
                  └────────┬─────────┘
                           │ IDLE 中断
                           ▼
                  osSemaphoreRelease()
                           │
                           ▼
                  Task_RS485 唤醒
                           │
                  ┌────────┴─────────┐
                  │ Modbus_Poll()    │
                  │  1. 提取帧数据    │
                  │  2. CRC16 校验    │
                  │  3. 地址匹配?     │
                  │  4. 功能码分发    │
                  │  5. 构造响应帧    │
                  │  6. HAL_UART_Tx  │
                  │     (DE 自动控制) │
                  └──────────────────┘
```

- **IDLE 中断**: Modbus RTU 帧间隔 ≥ 3.5 字符时间; UART IDLE 检测线路空闲, 天然满足帧边界识别
- **DMA**: 减少 CPU 中断频率, 一帧数据一次唤醒
- **信号量**: `osSemaphoreId_t s_rx_sem` — IDLE ISR 中释放, Task_RS485 中获取
- **DE 自动控制**: STM32H7 硬件 RS485 DE 模式, 无需手动 GPIO 翻转, 消除时序竞争

#### 5.3.4 Modbus CRC16 实现策略

STM32H753 内置 **CRC 硬件外设** (已在项目中启用 `crc.c`):
- 可配置多项式 (Modbus 需 0x8005, 初始值 0xFFFF, 输入/输出反转)
- 硬件加速: 一个时钟周期算一个 32-bit 字
- **推荐使用硬件 CRC** 替代软件查表法, 节省 ~512B ROM (查找表) + 加速计算

### 5.4 Transport_RS485 后端 (预留)

```c
/* Transport_HAL RS485 后端 — 通过 Modbus 控制指令间接实现 */
static const Transport_Ops_t s_rs485_ops = {
    .type   = TRANSPORT_RS485,
    .init   = rs485_init,      /* UART2 + DMA + IDLE 中断初始化 */
    .send   = rs485_send,      /* Modbus 响应帧发送 */
    .recv   = rs485_recv,      /* IDLE 中断 + 信号量阻塞接收 */
    .deinit = rs485_deinit,
};
```

注: RS485 通道主要用于 Modbus 寄存器交互, 非图像传输通道。图像上传始终走以太网。

---

## 6. 雷达增强方案 (V2.5 规划)

### 6.1 系统定位

雷达用于解决雨雾、低照度、逆光、雨滴遮挡等视觉触发不稳定问题。系统不把雷达作为图像识别替代物, 而是用于提供全天候的目标存在、距离、速度、方向和车道信息。

推荐分工:

| 层级 | 职责 |
|---|---|
| 雷达模块 | 点云/目标检测/跟踪, 输出目标列表 |
| `STM32H753ZI` | 解析目标列表, 抓拍区判断, 状态机, UDP/Modbus 转发 |
| 云端/上位机 | AI 识别, 复杂雷视融合, 事件判断 |
| `PLC/HMI` | 现场触发、状态读取、联锁和执行机构控制 |

### 6.2 不放在 H753 上的任务

| 任务 | 不建议原因 | 替代方式 |
|---|---|---|
| 雷达原始 ADC/点云处理 | 数据量和算法链路超出 MCU 稳定调度边界 | 选带目标输出的毫米波雷达模块 |
| 复杂多目标跟踪 | 持续占用 CPU/RAM, 影响图传和控制实时性 | 雷达模块或上位机跟踪 |
| 深度雷视融合 | 需要大模型、大内存和复杂同步 | 云端/上位机或独立 AI 模块 |
| 本地车牌/车型识别 | CNN/OCR 算力与内存压力高 | 云端/上位机/AI 模块 |

### 6.3 推荐业务流

```
Radar Target List
    └─ Task_Radar: parse + zone judge
        ├─ no target       -> RADAR_GUARD
        ├─ approaching     -> ARMED_STANDBY
        ├─ in capture zone -> ACTIVE_CAPTURE -> Task_Camera JPEG
        └─ radar timeout   -> FAULT_FALLBACK

Task_Camera -> FrameDesc_t + RadarMeta -> Task_NetTx -> Cloud/Upper Computer
Task_RS485  -> PLC/HMI reads target count, speed, range, fusion state
```

### 6.4 接口建议

优先选择 `UART/CAN/Ethernet` 输出目标列表的雷达模块。不要复用 `USART2 + MAX3485`, 因为该链路已经作为 `PLC/HMI Modbus RTU` 从站接口。若雷达必须使用串口, 量产板应单独预留一组 `RADAR_UART_TX/RX` 或 `FDCAN_RX/TX`。
---

## 7. 云平台集成路径 (远期)

> 本节为远期规划，不在 V2 首次迭代范围内。

### 6.1 分层策略

```
当前 V2 架构:
  App → Comm_Manager → Transport_ETH (Netconn UDP)

远期云平台:
  App → Comm_Manager → Protocol_MQTT (lwip netconn TCP)
                     → Transport_ETH
```

在 Transport 层之上再插入 Protocol 层 (MQTT Client)，不影响已有的 UDP 图传通道。

### 6.2 双通道并存

| 通道 | 协议 | 用途 |
|------|------|------|
| UDP (保留) | Raw UDP 分片 | 图传 — 高吞吐低延迟 |
| TCP (新增) | MQTT 3.1.1 | 信令 — 心跳、指令、设备影子、OTA 通知 |

两个通道共享同一个以太网物理链路，使用不同端口，互不干扰。

### 6.3 先决条件

- Netconn API 迁移完成 (V2 已实现)
- LwIP `lwipopts.h` 中启用 `LWIP_TCP=1` (当前可能仅启用 UDP)
- 评估 RAM 开销: TCP PCB (~200B) + MQTT 状态 (~1KB) + TLS 可选 (~40KB mbedtls)

---

## 8. 重构实施路线

### Phase 1: 生产者-消费者解耦 (最小改动, 最大收益)

**改动范围**: `freertos.c`, `Vision_Pipeline.c`

1. 在 D2 SRAM 中分配双缓冲 Buffer A/B
2. 新增 `osMessageQueue` 传递 `FrameDesc_t`
3. `Task_Camera` 采集完成后入队，立即切换缓冲区采集下一帧
4. `Task_Net` 从队列取帧并发送 (暂仍用 Raw API，后续迁移)
5. 实现缓冲区状态管理 (FREE/QUEUED/SENDING)

**验收标准**:
- 采集和发送在时间线上重叠执行
- JPEG 帧率从 ~14fps 提升至 ~25fps+
- 网络拥塞时 Camera 任务不阻塞，丢帧并打印告警

### Phase 2: LwIP Raw API → Netconn API

**改动范围**: `Net_Client.c`, `lwipopts.h`

1. 确认 `lwipopts.h` 中 `LWIP_NETCONN=1`
2. 将 `udp_new/udp_bind/udp_sendto` 替换为 `netconn_new/netconn_bind/netconn_send`
3. 将 `net_recv_cb` 回调替换为 `netconn_recv()` 阻塞接收
4. 发送使用 `netbuf_ref()` 保留零拷贝特性
5. 拆分 Task_Net 为 Task_NetTx + Task_NetRx

**验收标准**:
- 消除 Raw API 跨线程调用的竞态条件
- UDP 收发功能不变，Wireshark 抓包验证
- `netconn_recv()` 阻塞等待，不再 `osDelay(100)` 轮询

### Phase 3: Transport 抽象层

**改动范围**: 新增 `Transport_HAL.h/c`, `Transport_ETH.c`, 修改 `Net_Client.c`

1. 定义 `Transport_Ops_t` 虚表接口
2. 将现有 Netconn 逻辑封装为 `Transport_ETH`
3. 上层代码通过 `Transport_Send()` 调用，不感知底层通道
4. 为 RS485 预留 `Transport_RS485` 接口桩

### Phase 4: RS485 Modbus RTU 集成

**改动范围**: 新增 `Transport_RS485.c`, `Modbus_Slave.c`, CubeMX 配置新 UART

1. CubeMX 配置 UART + DMA + IDLE 中断
2. 实现 Modbus RTU 帧收发 (CRC16 + 功能码 02/03/06/16)
3. 实现寄存器表映射到系统状态
4. 新增 `Task_RS485`

### Phase 5: 雷达增强版 (V2.5)

**改动范围**: 新增 `Radar_Manager.c`, `Task_Radar`, 修改 `Power_Manager.c`, `Net_Client.c`, `Modbus_Slave.c`

1. 接入输出目标列表的毫米波雷达模块
2. 新增 `RadarTarget_t/RadarSummary_t`
3. `Power_Manager` 改为雷达事件驱动状态机
4. UDP 上报 `RadarMeta`, Modbus 暴露雷达状态寄存器
5. `Motion_Detect` 保留为 fallback, 从主链路移除

### Phase 6: 云平台信令层 (远期)

**改动范围**: 新增 `Protocol_MQTT.c`, `Task_MQTT`

1. 启用 LwIP TCP
2. 集成 MQTT Client (coreMQTT 或 lwip-mqtt)
3. 实现心跳、设备属性上报、指令下发、OTA 通知
4. 可选: mbedtls TLS 加密

---

**文档版本**: v1.1
**创建日期**: 2026-04-02
**更新日期**: 2026-04-26
**维护者**: IVCIS Team
