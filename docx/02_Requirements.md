# IVCIS 高速入口智能视觉终端 - 需求文档

## 需求 1：图像采集与 JPEG 传输

**用户故事：** 作为系统，我需要从 OV5640 摄像头采集 JPEG 压缩图像，并通过 UDP 上传至云端。

### 验收标准

1. WHEN 系统启动，THEN 系统 SHALL 初始化 OV5640 摄像头，配置为 JPEG 输出模式。
2. 系统 SHALL 配置 OV5640 内置 JPEG 编码器，质量参数默认 50%。
3. 系统 SHALL 使用 DCMI 接口 + DMA 接收完整 JPEG 帧，采用双缓冲策略避免覆盖。
4. 系统 SHALL 将 JPEG 缓冲区放置在 D2 SRAM (0x30000000)，并配置 MPU 为 Non-cacheable。
5. 系统 SHALL 通过 LwIP UDP 协议将 JPEG 数据发送至云端服务器。
6. 系统 SHALL 使用 `PBUF_ROM` 零拷贝发送，避免 memcpy 占用 CPU。
7. 系统 SHALL 在每个数据包中包含帧序号，便于云端追踪。
8. 系统 SHALL 记录发送日志 `[INFO] [Net] Frame #xxx sent, size=xxx bytes`。
9. IF 摄像头初始化失败，THEN 系统 SHALL 记录 `[ERROR] [Vision] Camera init failed`。
10. IF 网络发送失败超过 3 次，THEN 系统 SHALL 丢弃当前帧并记录 `[WARN] [Net] Frame dropped`。

> **设计决策**：使用 OV5640 内置 JPEG 而非 STM32 硬件 JPEG 编码器，因为 AI 推理在云端完成，边缘端无需 RGB 原图。

---

## 需求 2：云端协同与违规响应

**用户故事：** 作为系统，我需要接收云端 AI 推理结果（车型/车牌/违规标识），并在检测到违规时触发声光报警。

### 验收标准

1. 系统 SHALL 通过 UDP 接收云端返回的推理结果。
2. WHEN 云端返回"车型违规"标识，THEN 系统 SHALL 驱动 GPIO 输出高电平，点亮红色 LED (PB14)。
3. 系统 SHALL 同时驱动蜂鸣器发出间歇性警报声（1Hz 频率）。
4. 报警持续时间 SHALL 为 5 秒，之后自动停止。
5. 系统 SHALL 记录报警日志 `[WARN] [Alarm] Violation detected, ID: xxx`。
6. IF 系统处于低功耗模式，THEN 报警事件 SHALL 能够唤醒系统。

> **注意**：边缘端不做 AI 推理，所有车型/车牌识别在云端完成。

---

## 需求 3：帧差法运动检测与智能低功耗

**用户故事：** 作为系统，我需要在保持运动检测能力的前提下最大化节能，通过帧差法筛选有效帧，降低网络传输负担。

### 验收标准

1. 系统 SHALL 支持三级功耗模式：
   - **全速模式**：OV5640 JPEG 模式 @ 10fps，采集并上传云端
   - **轻活跃模式**：OV5640 JPEG 模式 @ 2fps，仅上传到云端
   - **帧差唤醒模式**：OV5640 切换为 160x120 灰度输出 @ 1fps，仅做帧差检测，不上传

2. 帧差唤醒模式下，系统 SHALL 将 OV5640 从 JPEG 模式切换为灰度输出模式。
3. WHEN 帧差法检测到运动变化超过阈值，THEN 系统 SHALL 在 200ms 内将 OV5640 切回 JPEG 模式并升级至全速模式。
4. IF 连续 30 秒无运动检测，THEN 系统 SHALL 从全速模式降级至轻活跃模式。
5. IF 连续 5 分钟无运动检测，THEN 系统 SHALL 降级至帧差唤醒模式。
6. 系统 SHALL 记录模式切换日志 `[INFO] [Power] Mode: FULL -> LIGHT`。
7. 帧差唤醒模式下，系统功耗 SHALL < 50mA。

> **方案决策**：帧差法需要未压缩图像数据。帧差唤醒模式下 OV5640 切换为灰度输出，检测到运动后切回 JPEG 模式。切换延迟 ~100ms 在"有无车辆"粒度下完全可接受。

---

## 需求 4：图像质量闭环控制

**用户故事：** 作为系统，我需要评估图像质量（亮度、模糊度），并通过 SCCB 动态调整 OV5640 参数，确保上传的图像始终可用。

### 验收标准

1. 系统 SHALL 每 10 帧计算图像平均亮度值。
2. IF 平均亮度 < 50（暗场景），THEN 系统 SHALL 增加曝光时间并提高增益。
3. IF 平均亮度 > 200（过曝场景），THEN 系统 SHALL 减少曝光时间并降低增益。
4. 系统 SHALL 使用 CMSIS-DSP 库计算 Laplacian 方差作为模糊度指标。
5. IF 模糊度低于阈值，THEN 系统 SHALL 标记该帧为 `LOW_QUALITY` 并在发送时附加标志位。
6. 系统 SHALL 通过 SCCB 协议动态修改 OV5640 曝光/增益寄存器。
7. 参数调整后，系统 SHALL 记录 `[DEBUG] [AutoExp] Adjusted: gain=xx, exp=xx, sharpness=xx`。

> **发挥 H753 特性**：使用 Cortex-M7 FPU + CMSIS-DSP 库（`arm_var_f32` 等）加速图像统计计算。

---

## 需求 5：舵机远程调角（运维功能）

**用户故事：** 作为运维人员，我希望通过上位机远程调整摄像头角度，无需到现场人工调整。

### 验收标准

1. 系统 SHALL 通过 TIM PWM 输出驱动舵机（50Hz 周期，0.5~2.5ms 脉宽对应 0°~180°）。
2. 系统 SHALL 通过 UDP 接收上位机发送的角度控制指令。
3. 指令格式 SHALL 包含目标角度（uint16_t, 0~180）。
4. WHEN 收到调角指令，THEN 系统 SHALL 暂停帧差检测，驱动舵机旋转至目标角度，等待稳定后恢复检测。
5. 系统 SHALL 限制舵机旋转速度，避免机械冲击。
6. 系统 SHALL 记录调角日志 `[INFO] [Servo] Angle adjusted to xxx°`。

> **设计决策**：舵机调角为运维模式功能，平时锁定不动。调角期间暂停帧差检测，避免画面移动导致误触发。

---

**文档版本**: v2.0  
**更新日期**: 2026-02-12  
**维护者**: IVCIS Team
