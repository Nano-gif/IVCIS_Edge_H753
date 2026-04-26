# IVCIS 工业智能相机 - 任务分解

> 当前任务表服务于 `03_Integrated_Project_Design.md` 中的雷达视觉融合方案。历史任务保留完成记录, 新增任务按当前方案继续推进。

## 当前实施顺序

1. `Task_Radar` + `Radar_Manager`: 框架已接入; 下一步替换真实雷达驱动, 生成 `RadarSummary`, 触发 `Camera_RequestCapture()`。
2. `Task_RS485` + Modbus RTU: 打通 `MAX3485`、PLC/HMI 状态读取和控制寄存器。
3. 事件证据链: 为每次雷达触发创建 `event_id`, 绑定多帧、雷达元数据和上传状态。
4. 未来工作: Edge AI 证据质量闭环, 包括图像质量评分、最佳帧选择、自动证据等级升级和镜头健康诊断。

---

## 里程碑 1-4: V1 基础功能 (已完成)

- [x] Task 1 - OV5640 初始化 (Vision_Pipeline.c)
- [x] Task 2 - OV5640 JPEG/灰度模式切换 (Vision_Pipeline.c)
- [x] Task 3 - LwIP UDP 初始化 (Net_Client.c)
- [x] Task 4 - UDP 零拷贝发送 (Net_Client.c)
- [x] Task 5 - 接收云端推理结果 (Net_Client.c)
- [x] Task 6 - 违规声光报警 (Alarm_Handler.c)
- [x] Task 7 - 自适应曝光 + 模糊度评估 (Auto_Exposure.c)
- [x] Task 8 - 帧差法运动检测 (Motion_Detect.c)
- [x] Task 9 - 三级功耗状态机 (Power_Manager.c)
- [x] Task 10 - 舵机 PWM 驱动 (Servo_Control.c)
- [x] Task 11 - 舵机远程指令集成
- [x] Task 12 - V1 系统集成联调

---

## 里程碑 5: 生产者-消费者并行化

---

- [x] Task 13 - 双缓冲帧管理 (Vision_Pipeline.c)

  **目标**: 将单缓冲 `s_raw_buf` 拆分为 Buffer A/B，实现 DMA 采集与网络发送的物理分离。

  **文件范围**: `APP/src/Vision_Pipeline.c`, `APP/Inc/app_config.h`

  **TDD 步骤**

  1. 测试: 两个缓冲区地址不重叠，均在 D2 SRAM 范围内。
  2. 测试: `Vision_CaptureStart()` 交替使用 Buffer A/B。
  3. 测试: 当前采集使用 Buffer A 时，Buffer B 的内容不被修改。
  4. 实现双缓冲分配与切换逻辑。

  **编码约束**: 缓冲区状态枚举 (FREE/QUEUED/SENDING)；D2_SRAM_SECTION + IVCIS_ALIGN_32。

---

- [x] Task 14 - 帧消息队列 (freertos.c)

  **目标**: 创建 `osMessageQueue` 传递 `FrameDesc_t`，连接 Task_Camera (生产者) 与 Task_NetTx (消费者)。

  **文件范围**: `Core/Src/freertos.c`, `APP/Inc/shared_types.h`

  **TDD 步骤**

  1. 测试: 队列创建成功，深度 = 2。
  2. 测试: Put 后 Get 能取到相同的 FrameDesc_t。
  3. 测试: 队列满时 Put 返回 osErrorResource (不阻塞)。
  4. 重构 `StartCameraTask`：采集完成后入队，不再调用 `Net_Client_SendImage`。
  5. 重构 `StartNetTask`：从队列取帧后调用发送。

  **编码约束**: `osMessageQueuePut` 使用 timeout=0 (非阻塞)；队列满时丢帧并打印告警。

---

- [x] Task 15 - 缓冲区生命周期管理

  **目标**: 实现缓冲区三态流转 (FREE→QUEUED→SENDING→FREE)，确保 DMA 不覆写正在发送的数据。

  **文件范围**: `APP/src/Vision_Pipeline.c`, `Core/Src/freertos.c`

  **TDD 步骤**

  1. 测试: 初始状态两个缓冲区均为 FREE。
  2. 测试: 入队后状态变为 QUEUED。
  3. 测试: NetTx 取出后状态变为 SENDING。
  4. 测试: 发送完成后状态恢复 FREE。
  5. 测试: 两个缓冲区都被占用时，Camera 丢帧不阻塞。

  **验收标准**: Wireshark 抓包确认帧率从 ~14fps 提升至 ~20fps+。

---

## 里程碑 6: LwIP Netconn API 迁移

---

- [x] Task 16 - lwipopts.h 配置检查

  **目标**: 确认 `LWIP_NETCONN=1`, `LWIP_TCPIP_CORE_LOCKING=1` 等宏已启用。

  **文件范围**: `LWIP/Target/lwipopts.h`

  **步骤**

  1. 检查并记录当前 `LWIP_NETCONN`, `LWIP_SOCKET`, `LWIP_TCP` 宏值。
  2. 启用 `LWIP_NETCONN=1` (如果未启用)。
  3. 验证编译通过。

---

- [x] Task 17 - Net_Client 迁移至 Netconn API

  **目标**: 将 `udp_new/udp_bind/udp_sendto/udp_recv` 替换为 Netconn 等价调用。

  **文件范围**: `APP/src/Net_Client.c`, `APP/Inc/Net_Client.h`

  **TDD 步骤**

  1. 测试: `Net_Client_Init()` 返回 0 (Netconn 创建 + bind 成功)。
  2. 测试: `Net_Client_SendFrame()` 使用 `netbuf_ref()` + PBUF_ROM 零拷贝。
  3. 测试: Wireshark 抓包确认 UDP 包格式不变。
  4. 删除 `net_recv_cb` 回调函数。
  5. 实现 `Net_Client_RecvCommand()` 使用 `netconn_recv()` 阻塞接收。

  **编码约束**: 移除 Raw API 全部引用；发送路径保留零拷贝。

---

- [x] Task 18 - Task_Net 拆分为 Task_NetTx + Task_NetRx

  **目标**: 发送和接收分离为独立任务，消除阻塞冲突。

  **文件范围**: `Core/Src/freertos.c`

  **步骤**

  1. 创建 `Task_NetTx` (AboveNormal, 8KB): 从帧队列取数据 → 发送。
  2. 创建 `Task_NetRx` (Normal, 4KB): `netconn_recv()` 阻塞等待 → 分发指令。
  3. 删除旧 `Task_Net`。
  4. 验证: 发送不影响接收，接收不阻塞发送。

---

## 里程碑 7: Transport 抽象层 + RS485

---

- [x] Task 19 - Transport_HAL 接口定义

  **目标**: 定义 `Transport_Ops_t` 虚表和 `Transport_Send/Recv` 通用 API。

  **文件范围**: 新增 `APP/Inc/Transport_HAL.h`, `APP/src/Transport_HAL.c`

  **步骤**

  1. 定义 `Transport_Ops_t` 结构体 (init/send/recv/deinit/mtu/name)。
  2. 实现 `Transport_Register()`, `Transport_Send()`, `Transport_Recv()` 路由函数。
  3. 将现有 Netconn 逻辑封装为 `Transport_ETH.c` 并注册。

---

- [ ] Task 20 - RS485 UART + DMA 底层驱动

  **目标**: CubeMX 配置 UART + DMA + IDLE 中断，实现不定长帧收发。

  **文件范围**: CubeMX `.ioc`, `Core/Src/usart.c`, 新增 `APP/src/RS485_Driver.c`

  **TDD 步骤**

  1. 确定 UART 引脚分配和 MAX485 DE/RE GPIO。
  2. CubeMX 配置 UART + DMA RX + IDLE 中断。
  3. 测试: 接收已知 Modbus 帧，验证长度和内容正确。
  4. 测试: DE/RE 方向切换时序正确 (发送前拉高，TC 中断后拉低)。

---

- [ ] Task 21 - Modbus RTU 从站协议实现

  **目标**: 实现 Modbus RTU 帧解析、CRC16 校验、功能码处理。

  **文件范围**: 新增 `APP/src/Modbus_Slave.c`, `APP/Inc/Modbus_Slave.h`

  **TDD 步骤**

  1. 测试: CRC16 计算结果与标准值一致。
  2. 测试: FC=0x03 读保持寄存器，返回正确数据。
  3. 测试: FC=0x06 写单寄存器，寄存器值更新。
  4. 测试: 非法地址返回异常码 0x02。
  5. 测试: CRC 错误静默丢弃。

---

- [ ] Task 22 - Modbus 寄存器表映射

  **目标**: 将 Modbus 寄存器表映射到系统状态变量和控制动作。

  **文件范围**: `APP/src/Modbus_Slave.c`

  **步骤**

  1. 状态监控区: 读取 PowerMgr_GetMode(), Net_Client_GetState() 等。
  2. 控制区: CMD_TRIGGER → 触发拍照, CMD_SERVO → Servo_SetAngle()。
  3. 参数配置区: 写入后更新 app_config 中的运行时参数。
  4. 详见 `archive/09_CommunicationProtocol.md` §3.3 寄存器表定义。

---

- [ ] Task 23 - Task_RS485 集成

  **目标**: 创建 RS485 FreeRTOS 任务，串联 UART 驱动 + Modbus 从站。

  **文件范围**: `Core/Src/freertos.c`

  **步骤**

  1. 创建 `Task_RS485` (Normal, 2KB)。
  2. 主循环: `Modbus_Poll()` (等待 IDLE 中断事件 → 解析 → 响应)。
  3. 测试: 使用 Modbus 调试工具 (ModbusPoll / pymodbus) 验证全部功能码。

---

## 里程碑 8: V2 系统集成联调

---

- [x] Task 24 - V2 端到端集成测试

  **目标**: 验证所有 V2 模块协同工作。

  **手动测试清单**

  1. 启动系统 → 双缓冲交替采集，帧率 ≥ 20fps。
  2. Wireshark 确认 UDP 分片包含正确的 NetChunkHdr 包头。
  3. 网络拥塞模拟 (拔网线 3 秒) → Camera 任务不阻塞, 恢复后自动续传。
  4. Modbus 调试工具: FC=0x03 读取状态寄存器。
  5. Modbus 调试工具: FC=0x06 写 CMD_TRIGGER=1 → 触发拍照并上传。
  6. Modbus 调试工具: FC=0x06 写 CMD_SERVO=90 → 舵机转至 90°。
  7. 功耗状态机: FULL→LIGHT→DETECT 自动降级, 运动唤醒。
  8. 同时进行: 以太网图传 + RS485 Modbus 查询, 互不干扰。

---

## 里程碑 9: 云平台信令 (远期)

---

- [ ] Task 25 - MQTT Client 集成
- [ ] Task 26 - 设备影子 / 属性上报
- [ ] Task 27 - OTA 固件升级通道

> 远期任务，依赖 V2 Netconn TCP 基础设施就绪。

---

## 里程碑 10: Edge AI 证据质量闭环 (未来工作)

---

- [ ] Task 28 - 图像质量评分与最佳帧选择

  **目标**: 雷达触发后连续抓拍 2~3 帧, 计算 `frame_quality_score`, 选择 `best_frame_id` 作为主证据帧。

  **文件范围**: 后续新增 `APP/src/Image_Quality_Manager.c`, `APP/Inc/Image_Quality_Manager.h`, 并接入 `Core/Src/freertos.c`。

- [ ] Task 29 - 事件证据等级自动升级

  **目标**: 依据最佳帧质量、雷达轨迹连续性、图像低质量原因和云端识别置信度, 自动决定上传最佳帧、辅助帧或诊断帧。

  **文件范围**: 后续新增 `APP/src/Event_Evidence.c`, `APP/Inc/Event_Evidence.h`。

- [ ] Task 30 - 镜头健康诊断

  **目标**: 识别镜头遮挡、污染、水滴、长期低亮度等设备健康问题, 通过 UDP 和 Modbus 上报维护状态。

  **说明**: 该里程碑不包含车辆存在二分类, 车辆存在和接近判断由雷达承担。

---

**文档版本**: v3.0
**更新日期**: 2026-04-02
**维护者**: IVCIS Team
**变更记录**:
- v3.0: 新增 Task 13-27 (V2 架构重构任务); 标记 V1 任务完成状态
- v2.0: 初始版本
