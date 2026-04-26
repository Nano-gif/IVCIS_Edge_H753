# IVCIS 工业智能相机 - 通信协议规格

> 本文档定义设备所有对外通信接口的协议格式、消息序列和交互规范。

---

## 1. 协议总览

本设备基础版支持两类对外通信通道。雷达增强版新增雷达外设输入, 但雷达不作为 PLC/HMI 总线使用, 只向 MCU 提供目标列表。

| 通道 | 物理层 | 协议 | 职责 | 带宽 |
|------|--------|------|------|------|
| **以太网** | LAN8742A RMII 100Mbps | UDP (图传+指令) + MQTT (远期) | 图像上传、云端控制指令、设备管理 | 高 |
| **RS485** | USART2 + MAX3485 | Modbus RTU | PLC/HMI 控制、参数读写、状态查询 | 低 (≤115.2kbps) |
| **Radar In** | 独立 UART/CAN/Ethernet | 厂商目标列表协议 | 车辆目标存在、距离、速度、方向、车道 | 中 |

### 双控制源架构

报警、功耗模式、拍照触发等执行动作可由**任一通道**触发:

```
云端 UDP 指令 (IVCIS_Command_t) ──┐
                                   ├──▶ Alarm_Set() / PowerMgr_ForceMode() / ...
RS485 Modbus 寄存器写入 ──────────┘
```

- **部署灵活**: 纯云端、纯本地 (RS485)、或两者并存均可
- **仲裁策略**: Last-Write-Wins (最后写入生效)，执行层不区分指令来源
- **典型部署 A**: 云端 AI → UDP 指令 → 相机执行报警/状态灯 (无 PLC)
- **典型部署 B**: PLC 产线节拍 → RS485 触发拍照 → 图像上传云端 → 云端结果直发 PLC (Modbus TCP)
- **典型部署 C**: 混合 — 云端下发报警指令, PLC 配置相机参数
- **典型部署 D**: 雷达触发抓拍 — 雷达目标进入抓拍区, MCU 上传 JPEG + RadarMeta, 云端进行雷视融合

---

## 2. 以太网 — UDP 图传协议

### 2.1 网络参数

| 参数 | 默认值 | 可配置 |
|------|--------|--------|
| 设备 IP | 192.168.1.10 | 是 (app_config.h / Modbus 寄存器) |
| 目标 IP | 192.168.1.100 | 是 |
| 上行端口 (设备→云端) | 8080 | 是 |
| 下行端口 (云端→设备) | 8000 | 是 |
| MTU | 1500 | 否 |
| 最大 UDP 载荷 | 1400 B | 否 (预留 IP/UDP 头) |

### 2.2 上行: 图像帧分片 (MCU → 云端)

单帧 JPEG 通常 20-80KB，超过 UDP 载荷上限，需要分片发送。

#### 分片包头格式 (NetChunkHdr_t, 20 字节)

```
偏移  大小  字段         说明
────────────────────────────────────────────────
0x00  4B    magic        固定 0x49564349 ("IVCI")
0x04  4B    frame_id     帧序号 (单调递增)
0x08  2B    chunk_idx    当前分片索引 (从 0 开始)
0x0A  2B    chunk_cnt    总分片数
0x0C  4B    total_size   原始 JPEG 总字节数
0x10  1B    flags        bit0: is_low_quality
                         bit1: alarm_active
                         bit2: radar_valid
                         bit3: radar_triggered
                         bit4-7: 保留
0x11  3B    reserved     填充对齐
────────────────────────────────────────────────
合计: 20 字节
```

#### 分片规则

```
每片载荷 = NET_MAX_UDP_PAYLOAD - sizeof(NetChunkHdr_t)
         = 1400 - 20 = 1380 字节 (纯 JPEG 数据)

分片数 = ceil(jpeg_size / 1380)

示例: 50KB JPEG → ceil(51200 / 1380) = 38 片
```

#### UDP 报文结构

```
┌──────────────┬─────────────────┐
│ NetChunkHdr  │  JPEG Data      │
│  (20 Bytes)  │ (≤1380 Bytes)   │
└──────────────┴─────────────────┘
```

> **V1 当前实现注意**: 当前代码 (`Net_Client.c`) 实际上**未附加包头**，直接发送裸 JPEG 分片 (PBUF_ROM 指向原始缓冲区)。上述包头格式定义于 `shared_types.h` 但未在发送路径中使用。V2 将正式启用包头。

#### 接收端重组逻辑 (云端侧)

1. 按 `frame_id` 聚合同一帧的所有分片
2. 按 `chunk_idx` 排序
3. 拼接所有分片的载荷部分
4. 验证拼接后大小 == `total_size`
5. 校验 JPEG SOI (0xFFD8) 和 EOI (0xFFD9) 标记

### 2.3 上行: 雷达元数据 (V2.5 规划)

雷达增强版建议把雷达目标信息作为独立元数据消息或随首片图像一起上报。为降低对图传分片格式的影响, 推荐新增 `RadarMeta_t`, 通过 `frame_id` 和 `timestamp_ms` 与 JPEG 帧关联。

#### 雷达目标元数据格式 (RadarMeta_t)

```
偏移  大小  字段              说明
────────────────────────────────────────────────
0x00  4B    magic             固定 0x49565244 ("IVRD")
0x04  4B    frame_id          对应 JPEG 帧序号
0x08  4B    timestamp_ms      MCU 时间戳
0x0C  4B    radar_track_id    雷达目标 ID
0x10  2B    range_cm          目标距离, 厘米
0x12  2B    speed_cms         目标速度, cm/s, 可为负
0x14  2B    azimuth_deg_x10   方位角, 0.1 度
0x16  1B    lane_id           车道编号
0x17  1B    confidence        置信度 0-100
0x18  1B    target_count      当前目标数量
0x19  1B    fusion_flags      bit0: in_capture_zone
                             bit1: approaching
                             bit2: radar_timeout
                             bit3: image_low_quality
0x1A  2B    reserved          对齐保留
────────────────────────────────────────────────
合计: 28 字节
```

上位机融合逻辑:
1. 按 `frame_id` 或 `timestamp_ms` 关联 `RadarMeta_t` 与 JPEG 帧。
2. 雷达侧提供目标存在、距离、速度、方向、车道。
3. 视觉侧提供图像证据、车牌、车型、颜色和图像质量。
4. 雨雾低质量场景下, 云端降低视觉识别权重, 以雷达目标确认作为触发依据。
### 2.4 下行: 云端指令 (云端 → MCU)

#### 指令包格式 (IVCIS_Command_t)

```
偏移  大小  字段              说明
────────────────────────────────────────────────
0x00  4B    magic             固定 0x49564352 ("IVCR")
0x04  1B    cmd_type          指令类型 (见下表)
0x05  3B    reserved          对齐填充
0x08  var   payload           指令负载 (按 cmd_type 解释)
────────────────────────────────────────────────
```

#### 指令类型定义

| cmd_type | 名称 | payload 格式 | 说明 |
|----------|------|-------------|------|
| 0x01 | CMD_TYPE_VIOLATION | `{uint8_t is_violation; char plate[16];}` | 违规识别结果 → 触发报警 |
| 0x02 | CMD_TYPE_SERVO | `{uint16_t angle_deg;}` | 舵机调角 (0-180°)，仅扩展版 |
| 0x03 | CMD_TYPE_POWER_MODE | `{uint8_t mode;}` | 强制功耗模式 (预留) |
| 0x04 | CMD_TYPE_TRIGGER | `{uint8_t reserved;}` | 触发单次拍照 (预留) |
| 0x05 | CMD_TYPE_FUSION_RESULT | `{uint8_t result; uint8_t confidence;}` | 云端雷视融合结果 (V2.5) |

> **扩展说明**: 当前固件仅实现 0x01 和 0x02。0x03/0x04 为预留指令，与 Modbus 控制区寄存器 (CMD_POWER_MODE / CMD_TRIGGER) 功能对等，使 UDP 通道具备与 RS485 同等的控制能力。基础量产版中舵机指令 (0x02) 默认不启用。

### 2.5 消息序列图

#### 正常拍照上传流程

```
  MCU                                  Cloud
   │                                     │
   │──── IVCI (frame=0, chunk 0/38) ────▶│
   │──── IVCI (frame=0, chunk 1/38) ────▶│
   │       ...                           │
   │──── IVCI (frame=0, chunk 37/38) ───▶│
   │                                     │ AI 推理
   │◀─── IVCR (cmd=0x01, violation) ─────│
   │                                     │
   │ Alarm_Set(true)                     │
   │                                     │
```

#### 舵机远程调角

```
  MCU                       Cloud/上位机
   │                            │
   │◀── IVCR (cmd=0x02, 90°) ──│
   │                            │
   │ Servo_SetAngle(90)         │
   │                            │
```

---

## 3. RS485 — Modbus RTU 协议

### 3.1 通信参数

| 参数 | 默认值 |
|------|--------|
| UART 外设 | USART2 (PD5-TX, PD6-RX, PD4-DE) |
| 收发器 | MAX3485 / SN65HVD75 (3.3V, 外接) |
| 波特率 | 9600 bps (可通过 Modbus 寄存器配置为 19200/38400/115200) |
| 数据位 | 8 |
| 校验 | 无 (None) |
| 停止位 | 1 |
| 从站地址 | 0x01 (可配置) |
| 帧间隔 | ≥ 3.5 字符时间 (9600bps 下 ≈ 4.06ms) |

### 3.2 支持的功能码

| 功能码 | 名称 | 说明 |
|--------|------|------|
| 0x02 | Read Discrete Inputs | 读取布尔状态量 |
| 0x03 | Read Holding Registers | 读取参数/状态寄存器 |
| 0x06 | Write Single Register | 写入单个参数 |
| 0x10 | Write Multiple Registers | 批量写入参数 |

### 3.3 保持寄存器表 (Holding Registers)

> 所有寄存器均为 16-bit unsigned，地址从 0x0000 起。

#### 设备信息区 (0x0000 - 0x000F, 只读)

| 地址 | 名称 | 说明 | 值域 |
|------|------|------|------|
| 0x0000 | DEVICE_ID | 设备型号标识 | 固定 0x4956 ("IV") |
| 0x0001 | FW_VER_MAJOR | 固件主版本 | 0-255 |
| 0x0002 | FW_VER_MINOR | 固件次版本 | 0-255 |
| 0x0003 | UPTIME_H | 运行时长高16位 (秒) | 0-65535 |
| 0x0004 | UPTIME_L | 运行时长低16位 (秒) | 0-65535 |

#### 状态监控区 (0x0010 - 0x001F, 只读)

| 地址 | 名称 | 说明 | 值域 |
|------|------|------|------|
| 0x0010 | POWER_MODE | 当前功耗模式 | 0=FULL, 1=LIGHT, 2=DETECT |
| 0x0011 | NET_STATE | 网络状态 | 0=IDLE, 1=READY, 2=SENDING, 3=ERROR |
| 0x0012 | FRAME_CNT_H | 采集帧计数高16位 | - |
| 0x0013 | FRAME_CNT_L | 采集帧计数低16位 | - |
| 0x0014 | TX_CNT_H | 发送帧计数高16位 | - |
| 0x0015 | TX_CNT_L | 发送帧计数低16位 | - |
| 0x0016 | BRIGHTNESS | 画面亮度 (×10 定点) | 0-2550 |
| 0x0017 | SHARPNESS | 清晰度 (×10 定点) | 0-65535 |
| 0x0018 | JPEG_SIZE_H | 最近帧大小高16位 (字节) | - |
| 0x0019 | JPEG_SIZE_L | 最近帧大小低16位 (字节) | - |
| 0x001A | RADAR_STATE | 雷达状态 | 0=OFF, 1=READY, 2=ACTIVE, 3=TIMEOUT, 4=ERROR |
| 0x001B | RADAR_TARGET_COUNT | 当前雷达目标数量 | 0-255 |
| 0x001C | RADAR_RANGE_CM | 最近目标距离 | 0-65535 cm |
| 0x001D | RADAR_SPEED_CMS | 最近目标速度, 有符号补码 | -32768~32767 cm/s |
| 0x001E | RADAR_LANE_ID | 最近目标车道 | 0-255 |
| 0x001F | FUSION_STATE | 融合/触发状态 | 0=IDLE, 1=ARMED, 2=CAPTURE, 3=FALLBACK |

#### 控制区 (0x0020 - 0x002F, 读写)

| 地址 | 名称 | 说明 | 值域 |
|------|------|------|------|
| 0x0020 | CMD_TRIGGER | 写 1 触发单次拍照 | 0/1, 写后自动清零 |
| 0x0021 | CMD_POWER_MODE | 强制设置功耗模式 | 0=FULL, 1=LIGHT, 2=DETECT, 0xFF=自动 |
| 0x0022 | CMD_SERVO_ANGLE | 舵机目标角度 (仅扩展版) | 0-180 |
| 0x0023 | CMD_ALARM | 报警控制 | 0=关, 1=开 |
| 0x0024 | CMD_REBOOT | 写 0xA55A 触发软复位 | 魔数校验 |
| 0x0025 | CMD_RADAR_ENABLE | 启用/禁用雷达触发 | 0=禁用, 1=启用 |
| 0x0026 | CMD_CAPTURE_ZONE | 抓拍区距离阈值 | cm, 写 0 表示使用默认 |

#### 参数配置区 (0x0030 - 0x003F, 读写, 掉电丢失)

| 地址 | 名称 | 说明 | 默认值 |
|------|------|------|--------|
| 0x0030 | CFG_SLAVE_ADDR | Modbus 从站地址 | 1 |
| 0x0031 | CFG_BAUDRATE | 波特率编码: 0=9600, 1=19200, 2=38400, 3=115200 | 0 |
| 0x0032 | CFG_DEST_IP_H | 目标 IP 高16位 (如 192.168 → 0xC0A8) | 0xC0A8 |
| 0x0033 | CFG_DEST_IP_L | 目标 IP 低16位 (如 1.100 → 0x0164) | 0x0164 |
| 0x0034 | CFG_DEST_PORT | UDP 目标端口 | 8080 |
| 0x0035 | CFG_JPEG_FPS | JPEG 模式目标帧率 | 10 |
| 0x0036 | CFG_FULL_TIMEOUT | FULL→LIGHT 超时 (秒) | 30 |
| 0x0037 | CFG_LIGHT_TIMEOUT | LIGHT→DETECT 超时 (秒) | 300 |
| 0x0038 | CFG_AE_TARGET | 目标亮度 (0-255) | 128 |
| 0x0039 | CFG_RADAR_TIMEOUT_MS | 雷达数据超时阈值 | 500 |
| 0x003A | CFG_CAPTURE_RANGE_CM | 抓拍区距离阈值 | 800 |
| 0x003B | CFG_RADAR_MIN_CONF | 雷达目标最小置信度 | 60 |

### 3.4 离散输入表 (Discrete Inputs)

| 地址 | 名称 | 说明 |
|------|------|------|
| 0x0000 | MOTION_ACTIVE | 当前是否检测到运动 |
| 0x0001 | CAM_READY | 摄像头初始化成功 |
| 0x0002 | ETH_LINK | 以太网链路 UP |
| 0x0003 | LOW_QUALITY | 当前图像低质量标记 |
| 0x0004 | ALARM_ACTIVE | 报警正在执行 |
| 0x0005 | SERVO_MOVING | 舵机正在运动 (仅扩展版) |
| 0x0006 | RADAR_TARGET_PRESENT | 雷达当前是否检测到目标 |
| 0x0007 | RADAR_IN_CAPTURE_ZONE | 雷达目标是否进入抓拍区 |
| 0x0008 | RADAR_FAULT | 雷达通信超时或异常 |
| 0x0009 | FUSION_CONFIRMED | 雷视融合结果有效 |

### 3.5 Modbus 帧格式

#### 请求帧 (主站 → 设备)

```
┌──────┬──────┬────────────┬───────┐
│ Addr │ Func │  Data      │ CRC16 │
│ 1B   │ 1B   │  N Bytes   │ 2B    │
└──────┴──────┴────────────┴───────┘
```

#### 示例: 读取功耗模式 (FC=0x03, 地址=0x0010, 数量=1)

```
请求: 01 03 00 10 00 01 85 CF
       │  │  ├────┤ ├────┤ ├────┤
       │  │  起始   寄存器  CRC16
       │  │  地址   数量
       │  功能码
       从站地址

响应: 01 03 02 00 00 B8 44
       │  │  │  ├────┤ ├────┤
       │  │  │  数据   CRC16
       │  │  字节数(2)
       │  功能码
       从站地址

数据 0x0000 = PWR_FULL
```

#### 示例: 触发拍照 (FC=0x06, 地址=0x0020, 值=0x0001)

```
请求: 01 06 00 20 00 01 48 01
响应: 01 06 00 20 00 01 48 01  (原样回显)
```

### 3.6 RS485 消息序列图

#### PLC 触发拍照 + 读取状态

```
  PLC (Master)                   MCU (Slave)
     │                               │
     │── FC06: TRIGGER=1 ──────────▶│
     │◀── Echo ────────────────────│  触发 Vision_CaptureStart()
     │                               │  等待帧完成...
     │         (延迟 ~100ms)         │
     │── FC03: Read NET_STATE ─────▶│
     │◀── NET_STATE=2 (SENDING) ───│
     │                               │  发送完成...
     │── FC03: Read NET_STATE ─────▶│
     │◀── NET_STATE=1 (READY) ─────│
     │                               │
```

#### HMI 参数配置

```
  HMI (Master)                   MCU (Slave)
     │                               │
     │── FC03: Read CFG区 ─────────▶│
     │◀── 当前配置参数 ──────────────│
     │                               │
     │── FC10: 批量写 CFG区 ───────▶│
     │◀── Echo ────────────────────│  应用新配置
     │                               │
```

#### 雷达触发抓拍 + 雷视融合 (V2.5)

```
  Radar Module             MCU Camera              Cloud/Upper Computer          PLC/HMI
      │                       │                            │                         │
      │── Target List ───────▶│                            │                         │
      │                       │ 判断进入抓拍区              │                         │
      │                       │── JPEG + RadarMeta ───────▶│                         │
      │                       │                            │ 雷视融合/AI识别          │
      │                       │◀── Fusion Result/Command ─│                         │
      │                       │                            │                         │
      │                       │── DO_ALARM/状态灯 ────────▶│                         │
      │                       │◀──── FC03: Read RADAR_* ─────────────────────────────│
      │                       │──── RADAR_STATE/RANGE/SPEED ───────────────────────▶│
```
---

## 4. 错误处理

### 4.1 UDP 层

| 错误 | 处理方式 | 日志 |
|------|---------|------|
| `netconn_send` 失败 | 连续 3 次失败丢弃当前帧 | `[WARN] [Net] Frame dropped` |
| 目标不可达 | 继续发送 (UDP 无连接) | `[WARN] [Net] Send err=%d` |
| 帧队列满 | 丢弃最新帧 | `[WARN] [Camera] Frame dropped: backpressure` |

### 4.2 Modbus 层

| 错误 | 处理方式 | 响应 |
|------|---------|------|
| CRC 校验失败 | 静默丢弃 (Modbus 标准) | 无响应 |
| 非法功能码 | 返回异常响应 | 异常码 0x01 |
| 非法寄存器地址 | 返回异常响应 | 异常码 0x02 |
| 写只读寄存器 | 返回异常响应 | 异常码 0x02 |
| 写入值超范围 | 返回异常响应 | 异常码 0x03 |

### 4.3 雷达输入层 (V2.5)

| 错误 | 处理方式 | 状态输出 |
|------|---------|---------|
| 雷达帧校验失败 | 丢弃当前帧, 保留上次有效目标 | `RADAR_STATE=ERROR` |
| 雷达数据超时 | 进入 `FAULT_FALLBACK`, 启用周期抓拍或视觉帧差 | `RADAR_STATE=TIMEOUT` |
| 目标置信度过低 | 不触发抓拍, 仅更新诊断计数 | `FUSION_STATE=IDLE` |
| 目标数量过多 | 只保留最近/最高置信目标给 MCU, 完整融合交给云端 | `RADAR_TARGET_COUNT` 保留实际数量 |

---

**文档版本**: v2.1
**创建日期**: 2026-04-02
**更新日期**: 2026-04-26
**维护者**: IVCIS Team
**变更记录**:
- v2.1: 新增雷达输入通道、RadarMeta 上行格式、Modbus 雷达状态寄存器、雷达触发抓拍消息序列和错误处理
- v2.0: 补充双控制源架构说明和典型部署场景; 对齐产品化定位 (舵机标记为扩展版); 补充 USART2/MAX3485 硬件信息; 预留 UDP 功耗模式/触发拍照指令
- v1.0: 初始版本
