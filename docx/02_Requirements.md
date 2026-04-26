# IVCIS 工业智能相机 - 需求文档

> **V2 更新**: 项目定位从高速入口终端演进为工业智能相机平台。新增并行采集传输、Modbus RTU 等需求。

---

## 需求 1: 图像采集与 JPEG 传输

**用户故事:** 作为系统，我需要从 OV5640 摄像头采集 JPEG 压缩图像，并通过 UDP 上传至云端。

### 验收标准

1. WHEN 系统启动，THEN 系统 SHALL 初始化 OV5640 摄像头，配置为 JPEG 输出模式。
2. 系统 SHALL 配置 OV5640 内置 JPEG 编码器，质量参数默认 50%。
3. 系统 SHALL 使用 100KB D2 区划分 **双缓冲 (Buffer A/B)**，实现采集与发送的物理分离与并行。
4. 系统 SHALL 在帧采集完成后，将 `FrameDesc_t` 通过 RTOS 消息队列传递给网络发送任务，并**立即切换到另一缓冲区开始采集下一帧**。
5. 系统 SHALL 采用 Drop 策略抵御网络拥塞: 若消息队列满，直接丢弃当前帧以确保采集永不阻塞。
6. 系统 SHALL 通过 LwIP **Netconn API** 配合 `netbuf_ref()` + `PBUF_ROM` 实现零拷贝发送。
7. 系统 SHALL 严格配置 MPU 将 D2 SRAM 标记为 Non-cacheable，防止 D-Cache 导致数据不一致。
8. 系统 SHALL 在 JPEG 分片前置入 `NetChunkHdr_t` 包头 (magic, frame_id, chunk_idx, chunk_cnt, total_size, flags)。
9. IF 摄像头初始化失败，THEN 系统 SHALL 记录 `[ERROR] [Vision] Camera init failed`。
10. IF 网络发送失败超过 3 次，THEN 系统 SHALL 丢弃当前帧并记录 `[WARN] [Net] Frame dropped`。

> **设计决策**: 使用 OV5640 内置 JPEG 而非 STM32 硬件 JPEG 编码器，因为 AI 推理在云端完成，边缘端无需 RGB 原图。

---

## 需求 2: 采集与传输并行 (V2 新增)

**用户故事:** 作为系统，我需要在网络发送当前帧的同时采集下一帧，实现流水线化，最大化帧率。

### 验收标准

1. 系统 SHALL 使用生产者-消费者模型: `Task_Camera` 为生产者，`Task_NetTx` 为消费者。
2. 系统 SHALL 通过 `osMessageQueue` 传递 `FrameDesc_t` (帧指针 + 长度 + 帧号)，队列深度 ≥ 2。
3. 每个缓冲区 SHALL 维护三态: FREE → QUEUED → SENDING → FREE。
4. Task_Camera SHALL 在入队后立即切换到 FREE 状态的缓冲区开始下一次采集。
5. WHEN 无可用 FREE 缓冲区，Task_Camera SHALL 丢弃当前帧并记录告警，不得阻塞等待。
6. 系统在 JPEG 模式下有效帧率 SHALL ≥ 20fps (640×480, 当前串行约 14fps)。

---

## 需求 3: 云端协同与违规响应

**用户故事:** 作为系统，我需要接收云端 AI 推理结果（车型/车牌/违规标识），并在检测到违规时触发声光报警。

### 验收标准

1. 系统 SHALL 通过 UDP 接收云端返回的 `IVCIS_Command_t` 指令包。
2. WHEN 云端返回 `CMD_TYPE_VIOLATION` 且 `is_violation=1`，THEN 系统 SHALL 驱动 GPIO 输出高电平，点亮红色 LED (PB14)。
3. 报警持续时间 SHALL 为 5 秒，之后自动停止。
4. 系统 SHALL 记录报警日志 `[WARN] [Alarm] Violation detected, plate: xxx`。
5. 接收任务 SHALL 使用 `netconn_recv()` 阻塞等待，不使用轮询。

---

## 需求 4: 帧差法运动检测与智能低功耗

**用户故事:** 作为系统，我需要在保持运动检测能力的前提下最大化节能。

### 验收标准

1. 系统 SHALL 支持三级功耗模式:
   - **FULL**: OV5640 JPEG @ 10fps，采集并上传
   - **LIGHT**: OV5640 JPEG @ 2fps，采集并上传
   - **DETECT**: OV5640 灰度 160×120 @ 1fps，仅帧差检测，不上传
2. DETECT 模式下，系统 SHALL 使用灰度帧差法检测运动。
3. WHEN 检测到运动，THEN 系统 SHALL 在 200ms 内升级至 FULL 模式。
4. IF 连续 30 秒无运动 → FULL → LIGHT。
5. IF 连续 5 分钟无运动 → LIGHT → DETECT。
6. 系统 SHALL 记录模式切换日志 `[INFO] [PWR] x->y`。

---

## 需求 5: 图像质量闭环控制

**用户故事:** 作为系统，我需要评估图像质量，并通过 SCCB 动态调整 OV5640 参数。

### 验收标准

1. 系统 SHALL 在 DETECT 灰度模式下计算画面平均亮度。
2. IF 亮度偏离目标 (128 ± 20)，THEN 系统 SHALL 通过 SCCB 步进调整 OV5640 曝光等级。
3. 系统 SHALL 计算简化 Laplacian 方差作为清晰度评分。
4. IF 清晰度低于阈值，THEN 系统 SHALL 标记帧为 `LOW_QUALITY`。
5. 系统 SHALL 记录调参日志 `[INFO] [AutoExp] Step Adjust -> Level x (B:xx.x)`。

---

## 需求 6: 舵机远程调角

**用户故事:** 作为运维人员，我希望通过云端或 RS485 远程调整摄像头角度。

### 验收标准

1. 系统 SHALL 通过 TIM4_CH4 PWM 驱动舵机 (50Hz, 0.5~2.5ms → 0°~180°)。
2. 系统 SHALL 接收 UDP 指令 (`CMD_TYPE_SERVO`) 或 Modbus 寄存器写入 (0x0022) 控制角度。
3. 角度范围校验 SHALL 限制在 0~180°，超范围截断。
4. 系统 SHALL 记录调角日志 `[INFO] [Servo] Angle adjusted to xxx°`。

---

## 需求 7: RS485 Modbus RTU 工业通信 (V2 新增)

**用户故事:** 作为 PLC/HMI 操作员，我需要通过 RS485 Modbus RTU 协议读取相机状态、配置参数、触发拍照。

### 验收标准

1. 系统 SHALL 作为 Modbus RTU **从站**，响应主站的寄存器读写请求。
2. 系统 SHALL 支持功能码: 0x02 (Read Discrete Inputs), 0x03 (Read Holding Registers), 0x06 (Write Single Register), 0x10 (Write Multiple Registers)。
3. 系统 SHALL 实现完整的寄存器表 (详见 archive/09_CommunicationProtocol.md §3.3):
   - 设备信息区: 型号、固件版本、运行时长 (只读)
   - 状态监控区: 功耗模式、网络状态、帧计数、亮度、清晰度 (只读)
   - 控制区: 触发拍照、设置功耗模式、舵机角度、报警开关 (读写)
   - 参数配置区: 从站地址、波特率、目标 IP/端口、帧率、超时参数 (读写)
4. 系统 SHALL 使用 UART IDLE 中断 + DMA 接收不定长 Modbus 帧。
5. 系统 SHALL 使用 CRC-16/MODBUS 校验，CRC 错误静默丢弃。
6. 系统 SHALL 对非法功能码、非法地址、非法数据值返回标准 Modbus 异常响应。
7. 写入 `CMD_TRIGGER=1` SHALL 触发一次拍照并通过以太网上传。
8. 默认参数: 从站地址=1, 波特率=9600, 8N1。

---

## 需求 8: LwIP 线程安全 (V2 新增)

**用户故事:** 作为系统，我需要在多任务环境下安全使用 LwIP 协议栈，消除竞态条件。

### 验收标准

1. 系统 SHALL 使用 LwIP **Netconn API** 替代 Raw API 进行 UDP 收发。
2. 所有 LwIP 操作 SHALL 通过 Netconn 内建的 mbox 机制在 tcpip_thread 上下文中执行。
3. 发送路径 SHALL 使用 `netbuf_ref()` 保留 PBUF_ROM 零拷贝特性。
4. 接收路径 SHALL 使用 `netconn_recv()` 阻塞式等待，不使用回调 + 轮询。
5. 系统 SHALL 拆分为独立的发送任务 (Task_NetTx) 和接收任务 (Task_NetRx)。

---

**文档版本**: v3.0
**更新日期**: 2026-04-02
**维护者**: IVCIS Team
**变更记录**:
- v3.0: 项目定位变更为工业智能相机; 新增需求 2 (并行采集传输), 需求 7 (RS485 Modbus), 需求 8 (Netconn 线程安全); 需求 1 更新为双缓冲+Netconn; 需求 6 新增 Modbus 控制方式
- v2.0: 初始版本
