# IVCIS 高速入口智能视觉终端 - 任务分解

> **编码规范**：所有代码须遵循 `coding_guidance.md`（Barr Group 安全规范 + ARM/Cortex-M 优化）

## 里程碑 1：摄像头采集基础

---

- [x] Task 1 - OV5640 初始化 (Vision_Pipeline.c)

  **目标**：验证 OV5640 能被正确初始化。

  **文件范围**：`App/Src/Vision_Pipeline.c`, `Drivers/BSP/ov5640/`

  **TDD 步骤**

  1. 测试：`Vision_Init()` 返回 0。
  2. 测试：读取 OV5640 芯片 ID (0x300A/0x300B) 应为 0x5640。
  3. 运行测试（预期失败）。
  4. 实现 `Vision_Init()`，配置 DCMI + OV5640 寄存器。

  **编码约束**：Opaque Pointer 封装 handle；I2C 指针 NULL check；所有类型用 `stdint.h`。

---

- [x] Task 2 - OV5640 JPEG 模式 + 灰度模式切换 (Vision_Pipeline.c)

  **目标**：实现 JPEG/灰度双模式切换，验证两种输出均正确。

  **文件范围**：`App/Src/Vision_Pipeline.c`, `Drivers/BSP/ov5640/`

  **TDD 步骤**

  1. 测试：`Vision_SetMode(VISION_MODE_JPEG)` 后，缓冲区前两字节为 0xFF 0xD8 (JPEG SOI)。
  2. 测试：`Vision_SetMode(VISION_MODE_GRAY_QVGA)` 后，帧大小应为 160×120 字节。
  3. 运行测试（预期失败）。
  4. 实现模式切换函数，通过 SCCB 配置 OV5640 寄存器。

  **编码约束**：DMA 缓冲区标记 `volatile`；D2 SRAM + MPU Non-cacheable。

---

## 里程碑 2：网络传输 + 零拷贝

---

- [ ] Task 3 - LwIP UDP 初始化 (Net_Client.c)

  **目标**：初始化 LwIP 协议栈，创建 UDP PCB 并绑定端口。

  **文件范围**：`App/Src/Net_Client.c`, `LWIP/Target/ethernetif.c`

  **TDD 步骤**

  1. 测试：`Net_Client_Init()` 返回 0。
  2. 测试：初始化后内部状态为 `NET_READY`。
  3. 运行测试（预期失败）。
  4. 实现 `Net_Client_Init()`。

  **编码约束**：无 malloc，使用 LwIP 静态内存池；ETH 描述符必须在 D2 SRAM。

---

- [ ] Task 4 - UDP 零拷贝发送 + 帧序号 (Net_Client.c)

  **目标**：用 PBUF_ROM 零拷贝发送 JPEG，Wireshark 可抓到包。

  **文件范围**：`App/Src/Net_Client.c`

  **TDD 步骤**

  1. 测试：`Net_Client_SendImage()` 后，`tx_frame_count` 递增。
  2. 手动验证：Wireshark 抓到 UDP 包，payload 前 4 字节为帧序号。
  3. 实现 `Net_Client_SendImage()`。

  **编码约束**：发送前 `SCB_CleanDCache_by_Addr()`；`pbuf_alloc` 返回 NULL check。

---

## 里程碑 3：云端协同 + 违规响应

---

- [ ] Task 5 - 接收云端推理结果 (Net_Client.c)

  **目标**：解析云端返回的 `IVCIS_Command_t`，区分违规结果和舵机指令。

  **文件范围**：`App/Src/Net_Client.c`

  **TDD 步骤**

  1. 测试：`Net_RecvResult()` 能解析 `cmd_type=0x01` 的违规结果。
  2. 测试：`Net_RecvResult()` 能解析 `cmd_type=0x02` 的舵机调角指令。
  3. 运行测试（预期失败）。
  4. 实现接收回调 + 解析函数。

  **编码约束**：结构体按大小降序排列；switch 必须有 default case。

---

- [ ] Task 6 - 违规声光报警 (Alarm_Handler.c)

  **目标**：根据云端返回的违规标识，驱动 GPIO 报警。

  **文件范围**：`App/Src/Alarm_Handler.c`

  **TDD 步骤**

  1. 测试：`Alarm_Trigger()` 后，PB14 (LD3) 输出高电平。
  2. 测试：`Alarm_Stop()` 后，所有报警 GPIO 低电平。
  3. 实现 `Alarm_Init()`、`Alarm_Trigger()`、`Alarm_Stop()`。

  **编码约束**：ISR 内仅置标志位，主循环处理报警逻辑 (Deferred Processing)。

---

## 里程碑 4：图像质量闭环

---

- [ ] Task 7 - 自适应曝光 + 模糊度评估 (Auto_Exposure.c)

  **目标**：使用 CMSIS-DSP 计算亮度和模糊度，通过 SCCB 动态调参。

  **文件范围**：`App/Src/Auto_Exposure.c`

  **TDD 步骤**

  1. 测试：输入暗图像 (avg < 50)，`AutoExp_Adjust()` 应增加曝光。
  2. 测试：输入亮图像 (avg > 200)，`AutoExp_Adjust()` 应减少曝光。
  3. 测试：输入模糊图像，`AutoExp_Analyze()` 返回 `is_low_quality = true`。
  4. 实现 `AutoExp_Analyze()` 和 `AutoExp_Adjust()`。

  **编码约束**：FPU 字面量用 `f` 后缀 (`3.14f`)；优先用 `arm_mean_f32`、`arm_var_f32`。

  **参考文档**：OV5640 曝光寄存器 (0x3500-0x3502)

---

## 里程碑 5：帧差法运动检测 + 低功耗

---

- [x] Task 8 - 帧差法运动检测 (Motion_Detect.c)

  **目标**：纯整数帧差法，用于低功耗模式下的运动判断。

  **文件范围**：`App/Src/Motion_Detect.c`

  **TDD 步骤**

  1. 测试：两帧相同，`Motion_Detect()` 返回 `false`。
  2. 测试：两帧差异 >10%，`Motion_Detect()` 返回 `true`。
  3. 运行测试（预期失败）。
  4. 实现 `Motion_Init()` 和 `Motion_Detect()`。

  **算法**

  ```c
  /* Pure integer frame diff - no float, no heap */
  uint32_t diff_count = 0;
  for (uint32_t i = 0; i < pixel_count; i++) {
      if (abs((int32_t)current[i] - (int32_t)previous[i]) > pixel_thresh) {
          diff_count++;
      }
  }
  return (diff_count > motion_thresh);
  ```

  **编码约束**：纯整数运算（无 FPU）；`static` 内部函数；`restrict` 指针参数。

---

- [ ] Task 9 - 三级功耗状态机 (Power_Manager.c)

  **目标**：实现事件驱动的功耗模式切换。

  **文件范围**：`App/Src/Power_Manager.c`

  **TDD 步骤**

  1. 测试：初始状态为 `POWER_MODE_FULL`。
  2. 测试：30秒无运动 → `POWER_MODE_LIGHT`。
  3. 测试：5分钟无运动 → `POWER_MODE_MOTION_ONLY`。
  4. 测试：`Motion_Detect()` 返回 `true` → `POWER_MODE_FULL`。
  5. 实现状态机。

  **模式定义**

  | 模式 | OV5640 输出 | 帧率 | 检测方式 | 功耗目标 |
  |------|-----------|------|----------|---------|
  | FULL | JPEG | 10fps | 上传云端 | ~200mA |
  | LIGHT | JPEG | 2fps | 上传云端 | ~100mA |
  | MOTION_ONLY | 灰度 160x120 | 1fps | 帧差法 | <50mA |

  **编码约束**：Active Object 状态机模式；无阻塞延时。

---

## 里程碑 6：舵机远程调角

---

- [ ] Task 10 - 舵机 PWM 驱动 (Servo_Control.c)

  **目标**：通过 TIM4_CH4 输出 PWM 驱动舵机旋转。

  **文件范围**：`App/Src/Servo_Control.c`

  **TDD 步骤**

  1. 测试：`Servo_Init()` 后，TIM4 正常运行。
  2. 测试：`Servo_SetAngle(90)` 后，PWM 脉宽为 1.5ms。
  3. 测试：`Servo_SetAngle(0)` → 0.5ms；`Servo_SetAngle(180)` → 2.5ms。
  4. 实现 `Servo_Init()` 和 `Servo_SetAngle()`。

  **编码约束**：角度范围校验 (0~180)；CCR 值用整数运算。

---

- [ ] Task 11 - 舵机远程指令集成 (Net_Client.c + Servo_Control.c)

  **目标**：接收云端 `cmd_type=0x02` 指令后驱动舵机调角。

  **文件范围**：`App/Src/Net_Client.c`, `App/Src/Servo_Control.c`

  **TDD 步骤**

  1. 测试：发送 `{cmd_type=0x02, angle=45}` 指令，舵机角度应变为 45°。
  2. 测试：调角期间，帧差检测应暂停。
  3. 实现指令分发逻辑。

---

## 里程碑 7：系统集成联调

---

- [ ] Task 12 - 系统集成联调

  **目标**：端到端验证所有模块协同工作。

  **文件范围**：`Core/Src/main.c`, 所有 `App/Src/*.c`

  **手动测试清单**

  1. 启动系统 → 摄像头 JPEG 采集正常。
  2. Wireshark 确认 UDP 包（PBUF_ROM 零拷贝）可达。
  3. 遮挡/移动物体 → 帧差法触发全速模式。
  4. 模拟违规信号 → LD3 (PB14) 红色报警。
  5. 过暗/过曝场景 → 自动调参日志输出。
  6. 上位机发送调角指令 → 舵机旋转到目标角度。
  7. 30秒无运动 → 降级至 LIGHT；5分钟 → MOTION_ONLY。
  8. MOTION_ONLY 模式下移动物体 → 200ms 内升级至 FULL。

  **编码约束**：`main.c` 遵循编排者模式——仅初始化 + 创建任务，不写业务逻辑。

---

**文档版本**: v2.0  
**更新日期**: 2026-02-12  
**维护者**: IVCIS Team
