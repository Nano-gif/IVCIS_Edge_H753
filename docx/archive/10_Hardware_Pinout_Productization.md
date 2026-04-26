# 硬件产品化接口与引脚规划

> 文档编号: 10 | 适用于芯片级量产主板规划 (§1-§9) 及 NUCLEO 转接板方案 (§10-§12)

## 1. 决策结论

### 1.1 基础量产版不建议上 SDRAM
当前项目以 `OV5640 + STM32H753ZI + Ethernet/UDP + RS485/Modbus RTU` 为主，现有软件已经按片内 SRAM 运行：双帧缓冲、`ETH DMA`、`LwIP Heap`、灰度检测缓冲均在当前内存布局内工作。因此基础量产版不建议增加 `SDRAM`。

仅在以下场景再评估 `SDRAM`：
- 本地 AI 推理需要更大的中间张量缓存
- 需要 `720p/1080p` 连续缓存、本地录像或多帧回放
- 需要 `RGB LCD` 帧缓冲或复杂 GUI

### 1.2 基础量产版不建议上 LCD
该产品更适合定义为“工业智能相机节点”，而不是本地人机界面终端。基础版建议无 `LCD`，现场查看、配置和维护通过 `Ethernet`、`RS485`、`SWD`、`Debug UART` 完成。

若后续有本地显示需求，优先做可选扩展板：
- 小尺寸 `SPI OLED/TFT` 优先
- 不建议基础版主板预留 `LTDC/FMC` 大规模显示电路

### 1.3 舵机/云台功能建议移出基础版
当前代码中 `PD15/TIM4_CH4` 用于舵机 PWM，适合作为验证功能或云台 SKU。基础工业量产版不建议保留板载舵机/云台接口，建议由 `PLC` 或外部运动控制器负责执行机构控制。

### 1.4 状态灯必须保留，但功能要重定义
- `PB14`：从“报警执行灯”调整为 `ALM/STATUS`
- `PB0`：保留为 `RUN`
- `PB7`：保留为 `NET`
- `PWR`：建议直接由电源良好信号或 `3V3` 常亮驱动，不占 MCU 引脚

### 1.5 雷达外设建议作为增强版预留
恶劣天气场景下, 单纯视觉帧差和图像识别容易受雨雾、逆光、低照度影响。建议在增强版中预留毫米波雷达外设接口, 但雷达模块应输出目标列表, 不把原始 ADC/点云处理放在 `STM32H753ZI` 上。基础版可不装雷达, 但 PCB 若空间和引脚允许, 建议预留 `RADAR_UART` 或 `FDCAN` 接口焊盘。

## 2. 产品职责边界

### 2.1 本产品负责
- 图像采集、压缩和上传
- 雷达增强版中接收目标列表, 做抓拍区判断和状态上报
- 识别结果、状态和告警信息输出
- `Ethernet` 到上位机/云端通信
- `RS485/Modbus RTU` 到 `PLC/HMI` 通信
- `DI/DO` 联动接口
- 板载状态指示

### 2.2 外部系统负责
- 舵机、云台、电机、气缸、继电器等执行器控制
- 设备联锁、时序和安全回路
- 多设备协同与停机逻辑
- 复杂雷视融合、本地车牌/车型识别和多目标跟踪

结论：基础量产版中，舵机/云台应删除；状态灯应保留。

## 3. 基础量产版应预留的接口

| 接口 | 是否必需 | 推荐电气形式 | 用途 |
|---|---|---|---|
| `Ethernet` | 必需 | `RJ45 + RMII PHY + ESD` | 图传、配置、远程维护 |
| `RS485` | 必需 | 2 线半双工，建议隔离 | `Modbus RTU` 到 `PLC/HMI` |
| `Camera` | 必需 | `OV5640 FPC/BTB` | 摄像头模组接口 |
| `SWD` | 必需 | Tag-Connect 或 `2x5 1.27mm` | 烧录、调试、产测 |
| `Debug UART` | 必需 | 3.3V TTL 串口 | 日志、维护 |
| `12~24V DC IN` | 必需 | 工业电源端子 | 系统供电 |
| `1DI` | 必需 | 光耦隔离输入 | 外部触发、节拍同步 |
| `1DO` | 必需 | 晶体管/光耦隔离输出 | 告警、识别结果输出 |
| `RESET/BOOT` | 必需 | 按键或焊盘 | 维护、产测 |
| `Radar` | 增强版建议预留 | 独立 UART/CAN/Ethernet | 雨雾/低照度下目标检测、速度/距离/方向 |
| `USB Device` | 可选 | USB FS/HS | 本地升级 |
| `TF/SD` | 可选 | `SDMMC/SPI` | 本地缓存/录像 |
| `LCD` | 可选 | 扩展板 | 本地显示，不进基础版 |

补充建议：
- 若更换为不带本振的摄像头模组，预留 `PA8/MCO1` 作为可选 `CAM_XCLK_OUT`
- `RS485`、`DI`、`DO` 优先按工业接口做浪涌、ESD、隔离设计
- 雷达接口不要复用 `USART2 + MAX3485`, 该链路应保留给 `PLC/HMI Modbus RTU`

## 4. 当前开发板到量产板的功能映射

| 当前功能 | 当前 MCU 引脚 | 量产板建议 | 结论 |
|---|---|---|---|
| 舵机 PWM | `PD15` | 基础版: 改为 `DO_ALARM`; 扩展版: 可选保留舵机 | 量产基础版不含舵机 |
| 报警 LED | `PB14` | 改为 `ALM/STATUS` | 保留，但从执行器改为状态灯 |
| 运行 LED | `PB0` | `RUN` | 保留 |
| 网络 LED | `PB7` | `NET` | 保留 |
| 用户按键 | `PC13` | 改为 `DI_TRIG` | 量产板重定义为外部触发输入 |
| 调试串口 | `PD8/PD9` | 保持不变 | 保留 |
| 摄像头 SCCB | `PB8/PB9` | 保持不变 | 保留，当前为 GPIO 模拟 SCCB |

## 5. MCU 引脚分配规划

### 5.1 必保留引脚

| 功能组 | 信号 | MCU 引脚 | 说明 |
|---|---|---|---|
| DCMI | `D0` | `PC6` | OV5640 数据 |
| DCMI | `D1` | `PC7` | OV5640 数据 |
| DCMI | `D2` | `PE0` | OV5640 数据 |
| DCMI | `D3` | `PE1` | OV5640 数据 |
| DCMI | `D4` | `PE4` | OV5640 数据 |
| DCMI | `D5` | `PB6` | OV5640 数据 |
| DCMI | `D6` | `PE5` | OV5640 数据 |
| DCMI | `D7` | `PE6` | OV5640 数据 |
| DCMI | `HSYNC` | `PA4` | 行同步 |
| DCMI | `VSYNC` | `PG9` | 场同步 |
| DCMI | `PIXCLK` | `PA6` | 像素时钟 |
| Camera Ctrl | `CAM_RST` | `PF2` | 摄像头复位 |
| Camera Ctrl | `CAM_PWDN` | `PF8` | 摄像头掉电 |
| SCCB | `SCL` | `PB8` | GPIO 模拟 SCCB |
| SCCB | `SDA` | `PB9` | GPIO 模拟 SCCB |
| ETH RMII | `REF_CLK` | `PA1` | PHY 50MHz 参考 |
| ETH RMII | `MDIO` | `PA2` | PHY 管理 |
| ETH RMII | `MDC` | `PC1` | PHY 管理 |
| ETH RMII | `CRS_DV` | `PA7` | RMII |
| ETH RMII | `RXD0` | `PC4` | RMII |
| ETH RMII | `RXD1` | `PC5` | RMII |
| ETH RMII | `TX_EN` | `PG11` | RMII |
| ETH RMII | `TXD0` | `PG13` | RMII |
| ETH RMII | `TXD1` | `PB13` | RMII |
| Debug UART | `TX` | `PD8` | 维护日志输出 |
| Debug UART | `RX` | `PD9` | 调试命令输入 |
| SWD | `SWDIO` | `PA13` | 下载调试 |
| SWD | `SWCLK` | `PA14` | 下载调试 |

### 5.2 量产化新增或重定义引脚

| 功能 | MCU 引脚 | 推荐复用 | 建议用途 | 备注 |
|---|---|---|---|---|
| `RS485_DE` | `PD4` | `USART2_RTS` 或 GPIO | 收发方向控制 | 建议在新版 `.ioc` 固定 |
| `RS485_TX` | `PD5` | `USART2_TX` | `Modbus RTU` 发送 | 产品主串口 |
| `RS485_RX` | `PD6` | `USART2_RX` | `Modbus RTU` 接收 | 产品主串口 |
| `DI_TRIG` | `PC13` | `GPIO/EXTI` | 外部触发输入 | 替代开发板按键 |
| `DO_ALARM` | `PD15` | `GPIO` | 外部告警输出 | 替代舵机 PWM |
| `ALM_LED` | `PB14` | `GPIO` | 告警/故障状态灯 | 量产板保留 |
| `RUN_LED` | `PB0` | `GPIO` | 运行状态灯 | 量产板保留 |
| `NET_LED` | `PB7` | `GPIO` | 网络状态灯 | 量产板保留 |
| `CAM_XCLK_OUT` | `PA8` | `MCO1` | 可选相机时钟输出 | 仅在新模组无本振时预留 |
| `RADAR_UART_TX` | 待定 | UART_TX | 雷达模块串口发送 | 增强版预留, 不占 `USART2` |
| `RADAR_UART_RX` | 待定 | UART_RX | 雷达模块串口接收 | 增强版预留, 不占 `USART2` |
| `RADAR_INT` | 待定 | GPIO/EXTI | 雷达数据就绪/目标事件 | 可选 |
| `RADAR_RST` | 待定 | GPIO | 雷达模块复位 | 可选 |

## 6. 接口到连接器的引脚/BOM 对照

### 6.1 连接器分配表

| 位号 | 接口 | 推荐连接器 | 关键信号/网络 | 对应 MCU 引脚 |
|---|---|---|---|---|
| `J1` | Ethernet | RJ45 带网络变压器 | `REF_CLK/MDIO/MDC/CRS_DV/RXD0/RXD1/TX_EN/TXD0/TXD1` | `PA1/PA2/PC1/PA7/PC4/PC5/PG11/PG13/PB13` |
| `J2` | DC IN | 2Pin/3Pin 工业端子 | `VIN+/VIN-/FG` | 电源输入，不直接进 MCU |
| `J3` | RS485 | 3Pin 端子 | `A/B/GND`, `DE` 由板内控制 | `PD4/PD5/PD6` |
| `J4` | SWD | Tag-Connect 或 `2x5 1.27mm` | `SWDIO/SWCLK/NRST/3V3/GND` | `PA13/PA14/NRST` |
| `J5` | Debug UART | 4Pin JST/PH | `TX/RX/3V3/GND` | `PD8/PD9` |
| `J6` | Camera | `FPC/BTB` | `D0~D7, HSYNC, VSYNC, PIXCLK, RST, PWDN, SCL, SDA, 3V3, 2V8, 1V8, GND` | `PC6/PC7/PE0/PE1/PE4/PB6/PE5/PE6/PA4/PG9/PA6/PF2/PF8/PB8/PB9` |
| `J7` | DI | 2Pin 端子 | `DI+/DI-` 或 `TRIG/GND` | `PC13` |
| `J8` | DO | 2Pin/3Pin 端子 | `DO_OUT/DO_GND` 或 `DO+/DO-` | `PD15` |
| `J9` | RESET/BOOT | 按键或焊盘 | `NRST/BOOT0/GND` | `NRST/BOOT0` |
| `J10` | Radar | 4~8Pin 端子/板对板 | `3V3/5V/GND/UART或CAN/INT/RST` | 待最终雷达模块确定 |

说明：
- `J6` 的实际 pin 序需要按选定 `OV5640` 模组 pinmap 最终落实，本文档先固定网络分配
- 当前工程未使用硬件 `I2C`，而是用 `PB8/PB9` 软件模拟 `SCCB`

### 6.2 PCB 关键器件 BOM

| 位号前缀 | 器件 | 数量 | 服务接口 | 选型备注 |
|---|---|---:|---|---|
| `U1` | `STM32H753ZIT6` | 1 | 主控 | 保持当前方案 |
| `U2` | `LAN8742A` 或同类 RMII PHY | 1 | Ethernet | 紧邻 RJ45，注意参考地和 ESD |
| `U3` | 工业级 `RS485` 收发器 | 1 | RS485 | 优先支持隔离或外挂隔离 |
| `U4` | 数字隔离器/隔离电源 | 1 | RS485/DI/DO | 按 EMC 与安规目标决定 |
| `U5` | `24V -> 5V` DCDC | 1 | 主电源 | 工业输入级 |
| `U6` | `5V -> 3.3V` DCDC/LDO | 1 | MCU/PHY | 视整机功耗选型 |
| `U7` | `3.3V -> 2.8V` LDO | 1 | Camera AVDD | 摄像头模拟电源 |
| `U8` | `3.3V -> 1.8V` LDO | 1 | Camera IOVDD/Core | 依模组要求 |
| `J10` | 雷达模块连接器 | 0/1 | Radar | 增强版装配, 基础版 DNP |
| `U9` | 雷达电源 LDO/DCDC | 0/1 | Radar | 按模块 3.3V/5V 电流决定 |
| `J1` | RJ45 带磁性器件 | 1 | Ethernet | 推荐工业温度范围 |
| `J2` | 工业电源端子 | 1 | 电源输入 | 支持锁线或防松脱 |
| `J3` | 3Pin 端子 | 1 | RS485 | 建议预留终端电阻拨码 |
| `J4` | SWD 调试座 | 1 | SWD | 产线和维护共用 |
| `J5` | UART 调试座 | 1 | Debug UART | 3.3V TTL |
| `J6` | Camera FPC/BTB | 1 | OV5640 | 按最终模组选型 |
| `J7` | 2Pin 端子 | 1 | DI | 建议前级光耦 |
| `J8` | 2Pin/3Pin 端子 | 1 | DO | 可接 PLC/塔灯/蜂鸣器 |
| `D1` | 电源 TVS | 1 | 电源入口 | 抗浪涌 |
| `D2` | Ethernet ESD 阵列 | 1 | RJ45 | 网口防护 |
| `D3` | RS485 TVS | 1 | RS485 | 总线防护 |
| `OK1` | 光耦输入 | 1 | DI | 建议隔离 |
| `Q1` | MOSFET/高边或低边驱动 | 1 | DO | 按外部负载选择 |
| `LED1~LED4` | 状态 LED | 3~4 | `PWR/RUN/NET/ALM` | `PWR` 建议不占 MCU 引脚 |

### 6.3 基础版明确不进入 BOM 的器件
- `SDRAM`
- `LCD` 模组、背光与 `LTDC/FMC` 配套电路
- 板载三针舵机接口 `5V/PWM/GND`
- 板载云台驱动电源

## 7. PCB 设计注意事项

- `DCMI`、`RMII`、相机时钟线优先短、直、少过孔
- 摄像头接口、`ETH PHY`、DCDC 周边要保证完整参考地和平面连续
- `RS485`、`DI`、`DO` 建议按工业接口做 ESD、浪涌、隔离和接地策略
- `PD15` 改为 `DO_ALARM` 后，应按工业数字输出设计，不按舵机 PWM 负载设计
- 基础版不为 `SDRAM`、`LCD` 预留大面积走线和电源，避免无效占板和 EMI 风险
- 雷达模块应远离 DCMI/RMII 高速线和相机模拟电源; 若使用天线一体模块, 需要保证正面净空和安装角度

## 8. 对软件工程的影响

按该硬件方案落地后，软件应同步调整：
- 将 `Alarm_Handler` 从“驱动业务报警灯”调整为“驱动状态灯 + 外部 DO”
- 将 `Servo_Control` 改为可裁剪模块，基础版默认关闭
- 在新版 `.ioc` 中启用 `USART2` 作为 `RS485`，固定 `PD4/PD5/PD6`
- 在需求与技术设计文档中，将“舵机/云台”降级为可选 SKU，而不是基础能力
- 雷达增强版中, 将 `Motion_Detect` 从主触发链路降级为 fallback
- 新增 `Task_Radar/Radar_Manager`, 并让 `Power_Manager` 基于雷达目标摘要驱动状态机

## 9. 最终建议

如果该相机要做成可量产工业产品，主板建议定位为：
- 无屏
- 无 `SDRAM`
- 固定视角或外部机构调节视角
- 以太网图传
- `RS485 Modbus RTU`
- `1DI + 1DO`
- `PWR/RUN/NET/ALM` 状态指示
- 增强版可选雷达接口, 用于恶劣天气目标检测

最关键的产品化动作有三项：
1. 删除基础版舵机/云台接口，`PD15` 改为 `DO_ALARM`
2. `PB14` 从报警执行灯改为 `ALM/STATUS`
3. 不在基础版 BOM 中引入 `SDRAM` 与 `LCD`
4. 雷达增强版只接收目标列表, 不让 `H753` 处理原始点云或复杂雷视融合

## 10. 适用范围说明

本文件 §1-§9 是按 **`STM32H753ZIT6` 芯片级量产主板** 规划的，不是按 `NUCLEO-H753ZI` 开发板直接规划的。其特点是：
- 直接以 MCU 引脚为中心定义接口
- 默认需要自建 `PHY/RJ45`、电源、相机电源和工业接口
- 面向后续量产 PCB

如果当前阶段是基于 `NUCLEO-H753ZI` 做联调或小批验证，应采用”**开发板 + 转接板**”方案。下面 §11-§12 即为该方案补充。详细的立创EDA原理图输入见 `11_LCEDA_Schematic_Input_NUCLEO_Adapter.md`。

## 11. 基于 NUCLEO-H753ZI 的转接板方案

### 11.1 总体思路
转接板不重复实现开发板已有资源，而是只补充项目真正缺失的部分：
- 复用开发板板载 `Ethernet RJ45 (CN14)`，不在转接板上重复做 `PHY/RMII`
- 复用开发板板载 `ST-LINK + Virtual COM Port`，不在转接板上重复做 USB 调试
- 通过 `CN11/CN12` ST morpho 连接器把相机、`RS485`、`DI/DO`、可选状态灯引出到转接板
- 转接板上补 `OV5640` 接口、电源 LDO、`RS485` 收发器、隔离 `DI/DO`、可选云台和 AI 扩展口

### 11.2 转接板推荐连接策略
- 主连接器使用 `CN11 + CN12`，因为 DCMI、GPIO、PWM、RS485 都能从 morpho 口拿到
- `Ethernet` 直接使用开发板 `CN14`
- 调试串口直接使用开发板 `ST-LINK VCP`
- 若要做 Arduino 兼容扩展，可额外兼容 `CN9/CN10`，但本项目不建议把核心相机链路放在 `CN7~CN10`

### 11.3 NUCLEO-H753ZI 上不建议转接板重复引出的资源
- `RMII` 引脚组：`PA1/PA2/PC1/PA7/PC4/PC5/PG11/PG13/PB13`
原因：这些已被板载 `LAN8742A + RJ45` 占用，应直接使用开发板网口
- `USART3 PD8/PD9`
原因：默认已接到 `ST-LINK VCP`，建议保留作调试日志，不在转接板上二次复用

## 12. NUCLEO-H753ZI 转接板 PCB 引脚连接与 BOM

### 12.1 开发板资源复用表

| 功能 | 开发板资源 | 是否在转接板重复实现 | 说明 |
|---|---|---|---|
| Ethernet | `CN14` 板载 RJ45 + PHY | 否 | 直接用开发板网口 |
| SWD 调试 | 板载 ST-LINK | 否 | 直接用开发板 |
| Debug UART | 板载 VCP (`USART3`) | 否 | 直接通过 USB 读日志 |
| 5V/3V3 供电 | `CN11/CN12` 电源脚 | 是，作为分配和二次稳压输入 | 转接板只做分配/稳压 |
| Camera 接口 | 无 | 是 | 转接板新增 FPC/BTB |
| RS485 | 无 | 是 | 转接板新增收发器和端子 |
| DI/DO | 无 | 是 | 转接板新增工业接口 |

### 12.2 转接板核心连接表

#### 12.2.1 Camera/OV5640 转接

| 转接板网络 | 开发板连接器 | 开发板管脚 | MCU 引脚 | 外部接口 |
|---|---|---|---|---|
| `CAM_D0` | `CN12-4` | `PC6` | `PC6` | `J_CAM` |
| `CAM_D1` | `CN12-19` | `PC7` | `PC7` | `J_CAM` |
| `CAM_D2` | `CN12-64` | `PE0` | `PE0` | `J_CAM` |
| `CAM_D3` | `CN11-61` | `PE1` | `PE1` | `J_CAM` |
| `CAM_D4` | `CN11-48` | `PE4` | `PE4` | `J_CAM` |
| `CAM_D5` | `CN12-17` | `PB6` | `PB6` | `J_CAM` |
| `CAM_D6` | `CN11-50` | `PE5` | `PE5` | `J_CAM` |
| `CAM_D7` | `CN11-62` | `PE6` | `PE6` | `J_CAM` |
| `CAM_HSYNC` | `CN11-32` | `PA4` | `PA4` | `J_CAM` |
| `CAM_VSYNC` | `CN11-63` | `PG9` | `PG9` | `J_CAM` |
| `CAM_PIXCLK` | `CN12-13` | `PA6` | `PA6` | `J_CAM` |
| `CAM_RST` | `CN11-52` | `PF2` | `PF2` | `J_CAM` |
| `CAM_PWDN` | `CN11-54` | `PF8` | `PF8` | `J_CAM` |
| `CAM_SCL` | `CN12-3` | `PB8` | `PB8` | `J_CAM` |
| `CAM_SDA` | `CN12-5` | `PB9` | `PB9` | `J_CAM` |
| `CAM_XCLK_OPT` | `CN12-23` | `PA8` | `PA8` | `J_CAM` 可选 |
| `CAM_3V3_IN` | `CN11-16` | `3V3` | - | 转接板电源输入 |
| `CAM_GND` | `CN11-8/19/20` | `GND` | - | 转接板地 |

说明：
- 转接板建议本地生成 `2.8V` 和 `1.8V` 供 `OV5640`
- 若所用摄像头模组自带时钟，可不接 `CAM_XCLK_OPT`

#### 12.2.2 RS485/DI/DO/状态指示

| 转接板网络 | 开发板连接器 | 开发板管脚 | MCU 引脚 | 外部接口 |
|---|---|---|---|---|
| `RS485_DE` | `CN11-39` | `PD4` | `PD4` | `U_RS485` |
| `RS485_TX` | `CN11-41` | `PD5` | `PD5` | `U_RS485` |
| `RS485_RX` | `CN11-43` | `PD6` | `PD6` | `U_RS485` |
| `RS485_A` | - | - | - | `U_RS485 pin6(A) -> J_RS485-1` |
| `RS485_B` | - | - | - | `U_RS485 pin7(B) -> J_RS485-2` |
| `DI_TRIG` | `CN11-23` | `PC13` | `PC13` | `J_DI` |
| `DO_ALARM` | `CN12-48` | `PD15` | `PD15` | `J_DO` |
| `LED_ALM` | `CN12-28` | `PB14` | `PB14` | `LED_ALM` |
| `LED_RUN` | `CN11-34` | `PB0` | `PB0` | `LED_RUN` |
| `LED_NET` | `CN11-21` | `PB7` | `PB7` | `LED_NET` |
| `A/B/GND` | - | - | - | `J_RS485` 端子 |

说明：
- 若采用 `MAX3485`，建议按 `SOP-8` 引脚定义连接：`1=RO`、`2=/RE`、`3=DE`、`4=DI`、`5=GND`、`6=A`、`7=B`、`8=VCC`。
- `PD5/PD6/PD4` 只对应 MCU 侧的 `TX/RX/DE`，并不直接等于外部总线的 `A/B`。
- `A/B` 应从 `U_RS485` 再引到 `J_RS485`，其中 `A -> J_RS485-1`，`B -> J_RS485-2`，`GND -> J_RS485-3`。
- `R_TERM` 应直接跨接 `A/B`，`D_RS485` 靠近端子放置；如需总线空闲偏置，建议预留一组 `A/B` 偏置电阻。
- 若仅做开发验证，可直接做 `PB14/PB0/PB7` 三个转接板 LED
- `DO_ALARM` 建议经晶体管或光耦后输出到端子，不要直接从 MCU 引脚出线

#### 12.2.2.1 MAX3485 SOP-8 引脚级连接

| MAX3485 引脚 | 引脚名 | 直接连接 | 备注 |
|---|---|---|---|
| `1` | `RO` | `CN11-43 -> PD6` | 网络名 `RS485_RX` |
| `2` | `/RE` | `CN11-39 -> PD4` | 与 `3-DE` 直接并网，网络名 `RS485_DE` |
| `3` | `DE` | `CN11-39 -> PD4` | 与 `2-/RE` 直接并网，网络名 `RS485_DE` |
| `4` | `DI` | `CN11-41 -> PD5` | 网络名 `RS485_TX` |
| `5` | `GND` | `GND` | 就近接地 |
| `6` | `A` | `J_RS485-1` | 网络名 `RS485_A`，同时接 `R_TERM`、`D_RS485` |
| `7` | `B` | `J_RS485-2` | 网络名 `RS485_B`，同时接 `R_TERM`、`D_RS485` |
| `8` | `VCC` | `3V3_IN` | 在 `8-VCC` 与 `5-GND` 之间放 `C_RS485=0.1uF` |

补充说明：
- 画图时请直接使用 `RO`、`/RE`、`DE`、`DI`、`GND`、`A`、`B`、`VCC` 这 8 个芯片引脚名，不要只写成笼统的“RS485 芯片”。
- `2-/RE` 与 `3-DE` 直接并网到 `PD4`，这是本项目当前的半双工控制方式。
- `A/B` 是总线侧差分引脚，必须从芯片脚位再引到端子，不要把 `PD5/PD6/PD4` 直接引到 `J_RS485`。

#### 12.2.3 转接板电源建议

| 电源网络 | 来源 | 用途 | 备注 |
|---|---|---|---|
| `5V_IN` | `CN11-18` 或外部 `5V` | RS485、可选舵机、部分外设 | 舵机负载建议外部单独供电 |
| `3V3_IN` | `CN11-16` | 逻辑电源输入 | 可直接供 RS485、LED、逻辑器件 |
| `2V8_CAM` | 转接板 LDO | OV5640 AVDD | 转接板本地生成 |
| `1V8_CAM` | 转接板 LDO | OV5640 IOVDD/Core | 转接板本地生成 |
| `SERVO_5V_EXT` | 外部端子 | 可选云台/舵机 | 不建议由开发板 USB 5V 直接带载 |

### 12.3 转接板 PCB BOM

| 位号前缀 | 器件 | 数量 | 用途 | 备注 |
|---|---|---:|---|---|
| `P1` | `2x35 2.54mm` 母座 | 1 | 对接 `CN11` | 建议堆叠型 |
| `P2` | `2x35 2.54mm` 母座 | 1 | 对接 `CN12` | 建议堆叠型 |
| `J_CAM` | `FPC/BTB` 相机座 | 1 | OV5640 模组 | 按最终模组选型 |
| `U_CAM1` | `3.3V -> 2.8V` LDO | 1 | Camera AVDD | 低噪声优先 |
| `U_CAM2` | `3.3V -> 1.8V` LDO | 1 | Camera IOVDD/Core | 低噪声优先 |
| `U_RS485` | 3.3V `RS485` 收发器 | 1 | Modbus RTU | 推荐 `MAX3485/SN65HVD75` 类 |
| `C_RS485` | `0.1uF` 去耦电容 | 1 | RS485 | 紧贴 `U_RS485` 的 `VCC/GND` |
| `D_RS485` | RS485 TVS | 1 | 总线防护 | 靠近端子 |
| `R_TERM` | `120R` 终端电阻 | 1 | RS485 | 建议跳线或拨码使能 |
| `R_BIAS_A/B` | 总线偏置电阻组 | 1 组 | RS485 | 可选预留，按网络拓扑装配 |
| `J_RS485` | 3Pin 端子 | 1 | `A/B/GND` | 工业端子 |
| `OK1` | 光耦输入 | 1 | `DI_TRIG` | 可选隔离 |
| `J_DI` | 2Pin 端子 | 1 | 外部触发输入 | 工业端子 |
| `Q_DO` | MOSFET/晶体管驱动 | 1 | `DO_ALARM` 输出级 | 按负载选型 |
| `J_DO` | 2Pin/3Pin 端子 | 1 | 外部告警输出 | 工业端子 |
| `LED_ALM` | 红色 LED | 1 | 告警状态 | 转接板状态灯 |
| `LED_RUN` | 绿色 LED | 1 | 运行状态 | 转接板状态灯 |
| `LED_NET` | 黄色/蓝色 LED | 1 | 网络状态 | 转接板状态灯 |
| `R_LEDx` | LED 限流电阻 | 3 | LED 限流 | 常规值 |
| `C_BULK` | 电源缓冲电容 | 2~4 | 5V/3.3V 去耦 | 靠近接口和 LDO |
| `C_DECx` | 0.1uF 去耦 | 若干 | 数字去耦 | 就近放置 |

### 12.4 转接板 PCB 设计建议
- Camera FPC 放在靠近 `CN11/CN12` 一侧，优先缩短 `DCMI` 线长
- `RS485` 端子和 TVS 靠板边，便于接线和浪涌泄放
- `DI/DO` 与 Camera 数字总线分区，避免噪声串扰
- 舵机/云台如保留可选接口，必须单独电源区域，不与 Camera 模拟电源混在一起

## 12. 可选扩展方案

> 以下方案均为可选，基础版默认不包含。扩展版作为独立 SKU 或子板。

### 12.0 雷达增强版
目标：解决雨雾、低照度、逆光等场景下视觉触发不稳定问题。雷达模块负责目标检测和跟踪, `H753` 只读取目标列表并触发抓拍。

推荐接口：
- `UART`: 低成本, 适合目标列表输出
- `CAN/FDCAN`: 工业抗干扰更好, 适合现场设备
- `Ethernet`: 高端雷达模块可选, 但会增加网络拓扑复杂度

不推荐：
- 复用 `USART2 + MAX3485`, 因为该链路已经用于 `PLC/HMI Modbus RTU`
- 让 `STM32H753ZI` 直接处理雷达原始 ADC 或点云

转接板建议新增：
- `J_RADAR` 连接器, 至少预留 `3V3/5V/GND/RADAR_TX/RADAR_RX/RADAR_INT/RADAR_RST`
- 雷达独立电源滤波和 ESD 保护
- 若使用天线一体模块, PCB 与结构件需预留前向净空
### 12.1 Edge AI Lite
目标：不增加第二颗主控，只在 `STM32H753ZI` 上跑轻量模型。

建议做法：
- 在转接板上预留 `QSPI`/模型存储接口
- 继续使用 `X-CUBE-AI`
- 适合小型分类、目标存在性判断、简单缺陷检测

推荐预留信号：

| 扩展网络 | 开发板连接器 | MCU 引脚 | 用途 |
|---|---|---|---|
| `AI_QSPI_NCS` | `CN12-70` | `PG6` | QSPI Flash/NOR |
| `AI_QSPI_CLK` | `CN12-22` | `PB2` | QSPI Clock |
| `AI_QSPI_IO0` | `CN12-45` | `PD11` | QSPI Data |
| `AI_QSPI_IO1` | `CN12-43` | `PD12` | QSPI Data |
| `AI_QSPI_IO2` | `CN11-46` | `PE2` | QSPI Data |
| `AI_QSPI_IO3` | `CN12-41` | `PD13` | QSPI Data |

适用场景：
- 图像预处理 + 小模型推理
- 云端识别前的本地筛选
- 断网情况下的本地告警

### 12.2 Edge AI Pro
目标：增加独立 AI 模块或加速器，而不是把所有推理压在 `H753` 上。

推荐结构：
- `H753` 负责采图、通信、控制
- 外挂 AI 模块负责推理
- 两者通过 `UART/SPI/QSPI/USB` 中的一种方式交换帧或结果

转接板建议新增：
- `J_AI` 板对板连接器，至少预留 `3V3/5V/GND/UART_TX/UART_RX/INT/RST`
- 若要走高速模型或图像数据，再追加 `QSPI` 组

推荐用途：
- YOLO 级目标检测
- OCR/车牌识别本地化
- 多目标跟踪或复杂场景分类

工程建议：
- 先做 `AI Lite` 版本验证系统闭环
- 只有当 `H753` 算力或内存不足时，再上 `AI Pro`

### 12.3 单轴舵机验证版
目标：保留当前软件里的 `Servo_Control`，方便实验验证。

建议连接：

| 网络 | 开发板连接器 | MCU 引脚 | 外部接口 |
|---|---|---|---|
| `GIMBAL_PWM1` | `CN12-48` | `PD15` | `J_SERVO1` |
| `SERVO_5V` | 外部端子 | - | `J_SERVO1` |
| `GND` | `CN11-19/20` | - | `J_SERVO1` |

注意：
- 舵机电源不要从开发板 USB 5V 直接取
- 建议单独加 `5V/2A` 以上外部输入

### 12.4 多轴云台扩展版
目标：在转接板上扩展多路 PWM，支持双轴或三轴云台。

推荐 PWM 引脚：

| 网络 | 开发板连接器 | MCU 引脚 | 定时器 |
|---|---|---|---|
| `GIMBAL_PWM1` | `CN12-52` | `PE9` | `TIM1_CH1` |
| `GIMBAL_PWM2` | `CN12-56` | `PE11` | `TIM1_CH2` |
| `GIMBAL_PWM3` | `CN12-55` | `PE13` | `TIM1_CH3` |

推荐做法：
- 基础版默认不焊接该部分
- 云台版作为单独 SKU 或可选子板
- 若进入工业现场版本，优先改为“相机给控制字，外部运动控制器执行”

### 12.5 扩展 BOM 增补

| 位号前缀 | 器件 | 数量 | 适用方案 | 备注 |
|---|---|---:|---|---|
| `J_RADAR` | 4~8Pin 连接器 | 0/1 | 雷达增强版 | 基础版 DNP |
| `U_PWR_RADAR` | LDO/DCDC | 0/1 | 雷达增强版 | 按模块供电要求选型 |
| `TVS_RADAR` | ESD/TVS | 0/1 | 雷达增强版 | 靠近接口 |
| `J_AI` | 板对板连接器 | 1 | `AI Pro` | 1.27mm 优先 |
| `U_QSPI` | QSPI NOR Flash | 1 | `AI Lite` | 模型存储 |
| `J_SERVO1` | 3Pin 舵机座 | 1 | 单轴云台 | `PWM/5V/GND` |
| `J_GIMBAL` | 4~8Pin 端子/排针 | 1 | 多轴云台 | 多路 PWM |
| `U_PWR_SERVO` | `5V` DCDC | 1 | 云台扩展 | 独立于开发板供电 |
| `TVS_SERVO` | TVS/反灌保护 | 1 | 云台扩展 | 保护 PWM 和电源 |
