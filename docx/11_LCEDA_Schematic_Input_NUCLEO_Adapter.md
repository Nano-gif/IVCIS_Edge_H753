# 11. 立创EDA画图输入（NUCLEO-H753ZI 转接板）

## 1. 适用范围

本文用于 `NUCLEO-H753ZI + 转接板` 方案，不是 `STM32H753ZIT6` 芯片级量产主板。转接板复用开发板板载 `RJ45 + PHY + ST-LINK VCP`，只补充以下功能：

- `OV5640` 相机接口
- `RS485 Modbus RTU`
- `1DI + 1DO`
- `RUN/NET/ALM` 状态灯
- 可选 `Edge AI` 与云台扩展

## 2. 可直接贴给立创EDA/原理图工程师的输入正文

```text
请绘制一块用于 ST NUCLEO-H753ZI 的功能转接板原理图。该转接板通过两只 2x35、2.54mm 母座分别对插开发板的 CN11 和 CN12，不重复实现开发板已有的 Ethernet PHY、RJ45、ST-LINK 和 USB 调试，仅实现 Camera、RS485、DI/DO、状态灯和可选扩展接口。

整套原理图分 5 个 sheet：

Sheet1_BaseConn_Power
1. 放置 P1、P2，器件为 2x35 2.54mm Female Header，分别对应 NUCLEO 的 CN11、CN12。
2. 从 P1 引出 3V3_IN、5V_IN、GND。3V3_IN 用于逻辑和相机 LDO 输入，5V_IN 仅用于低功耗外设；若装舵机，必须再引入外部 SERVO_5V_EXT。
3. P1-16 命名为 3V3_IN；P1-18 命名为 5V_IN；P1-8、P1-19、P1-20 统一接 GND。
4. 预留测试点 TP_3V3、TP_5V、TP_GND。

Sheet2_Camera_OV5640
1. 放置 J_CAM，作为 OV5640 模组连接器，封装按最终模组选型确定，可先用 24P FPC 0.5mm 占位。
2. 放置 U_CAM1：3.3V 转 2.8V LDO，输出 2V8_CAM。
3. 放置 U_CAM2：3.3V 转 1.8V LDO，输出 1V8_CAM。
4. 放置 CAM_SCL、CAM_SDA 上拉电阻，各 4.7k 到 3V3_IN。
5. 连接关系：
   CAM_D0=P2-4，CAM_D1=P2-19，CAM_D2=P2-64，CAM_D3=P1-61，CAM_D4=P1-48，CAM_D5=P2-17，CAM_D6=P1-50，CAM_D7=P1-62。
   CAM_HSYNC=P1-32，CAM_VSYNC=P1-63，CAM_PIXCLK=P2-13。
   CAM_RST=P1-52，CAM_PWDN=P1-54。
   CAM_SCL=P2-3，CAM_SDA=P2-5。
   可选 CAM_XCLK_OPT=P2-23。
6. J_CAM 同时引出 3V3_IN、2V8_CAM、1V8_CAM、GND。

Sheet3_RS485_DI_DO_LED
1. 放置 U_RS485，采用 3.3V RS485 收发器，优先 MAX3485 或 SN65HVD75。
2. 放置 J_RS485，3Pin 端子，定义为 A、B、GND。
3. 放置 D_RS485 作为 A/B 总线 TVS；预留 R_TERM=120R，可做跳线或拨码使能。
4. 如果 U_RS485 采用 MAX3485 SOP-8，请直接按芯片引脚名连接：
   U_RS485-1 `RO` -> `RS485_RX` -> P1-43(`PD6`)
   U_RS485-2 `/RE` -> `RS485_DE` -> P1-39(`PD4`)
   U_RS485-3 `DE` -> `RS485_DE` -> P1-39(`PD4`)
   U_RS485-4 `DI` -> `RS485_TX` -> P1-41(`PD5`)
   U_RS485-5 `GND` -> `GND`
   U_RS485-6 `A` -> `RS485_A` -> J_RS485-1
   U_RS485-7 `B` -> `RS485_B` -> J_RS485-2
   U_RS485-8 `VCC` -> `3V3_IN`
5. 在 U_RS485-8 `VCC` 与 U_RS485-5 `GND` 之间放置 `C_RS485=0.1uF` 去耦电容，尽量贴近芯片。
6. `R_TERM=120R` 直接跨接 U_RS485-6 `A` 与 U_RS485-7 `B`，建议跳线或拨码使能，仅在线路末端装配。
7. `D_RS485` 按所选 TVS 型号接在 `RS485_A`、`RS485_B`、`GND` 之间，优先靠近 `J_RS485`。
8. `J_RS485-1=A`，`J_RS485-2=B`，`J_RS485-3=GND`。
9. 可选预留一组总线偏置电阻；若装配偏置，直接围绕 U_RS485-6 `A` 和 U_RS485-7 `B` 实施，不要画在 MCU 侧。
10. 放置 OK1 作为 DI 光耦输入，外部端子 J_DI 为 2Pin，输出逻辑网名 DI_TRIG，接 P1-23；DI_TRIG 侧加 10k 上拉到 3V3_IN。
11. 放置 Q_DO 作为 DO_ALARM 驱动，可做开集电极或低边 MOSFET 输出；J_DO 为 2Pin 或 3Pin 端子。DO_ALARM 来自 P2-48，不允许直接把 MCU 引脚裸接到端子。
12. 放置三只状态灯：LED_ALM、LED_RUN、LED_NET，各串 1k 电阻到 3V3_IN 或按灌电流方式接法绘制，控制网分别来自 P2-28、P1-34、P1-21。

Sheet4_Optional_AI
1. 该页默认 DNP，用于边缘 AI 扩展。
2. 预留一颗 U_QSPI（QSPI NOR Flash）或一个 J_AI 板对板连接器。
3. 连接关系：
   AI_QSPI_NCS=P2-70，AI_QSPI_CLK=P2-22，AI_QSPI_IO0=P2-45，AI_QSPI_IO1=P2-43，AI_QSPI_IO2=P1-46，AI_QSPI_IO3=P2-41。
4. J_AI 至少预留 3V3、5V、GND、UART_TX、UART_RX、AI_INT、AI_RST。

Sheet5_Optional_Gimbal
1. 该页默认 DNP，用于验证版云台/舵机，不进入基础工业版。
2. 单轴方案：J_SERVO1 引出 GIMBAL_PWM1、SERVO_5V_EXT、GND，其中 GIMBAL_PWM1=P2-48。
3. 多轴方案：J_GIMBAL 引出 GIMBAL_PWM1=P2-52、GIMBAL_PWM2=P2-56、GIMBAL_PWM3=P2-55，再加 SERVO_5V_EXT 和 GND。
4. 舵机电源必须来自独立外部 5V 电源，不得直接由 NUCLEO USB 5V 供电。

统一要求：
1. 网络名按本文定义，不要自行改名。
2. Camera 数字线尽量短直，FPC 靠近 P1/P2。
3. RS485、DI/DO 放在板边，方便接线和浪涌防护。
4. Edge AI、云台相关器件默认标记为 DNP。
```

## 3. 关键网络对照表

### 3.1 Camera 网络

| 网络名 | NUCLEO 连接器 | MCU 引脚 | 外部接口 |
|---|---|---|---|
| `CAM_D0` | `P2-4` | `PC6` | `J_CAM` |
| `CAM_D1` | `P2-19` | `PC7` | `J_CAM` |
| `CAM_D2` | `P2-64` | `PE0` | `J_CAM` |
| `CAM_D3` | `P1-61` | `PE1` | `J_CAM` |
| `CAM_D4` | `P1-48` | `PE4` | `J_CAM` |
| `CAM_D5` | `P2-17` | `PB6` | `J_CAM` |
| `CAM_D6` | `P1-50` | `PE5` | `J_CAM` |
| `CAM_D7` | `P1-62` | `PE6` | `J_CAM` |
| `CAM_HSYNC` | `P1-32` | `PA4` | `J_CAM` |
| `CAM_VSYNC` | `P1-63` | `PG9` | `J_CAM` |
| `CAM_PIXCLK` | `P2-13` | `PA6` | `J_CAM` |
| `CAM_RST` | `P1-52` | `PF2` | `J_CAM` |
| `CAM_PWDN` | `P1-54` | `PF8` | `J_CAM` |
| `CAM_SCL` | `P2-3` | `PB8` | `J_CAM` |
| `CAM_SDA` | `P2-5` | `PB9` | `J_CAM` |
| `CAM_XCLK_OPT` | `P2-23` | `PA8` | `J_CAM` 可选 |

### 3.2 工业接口与状态指示

| 网络名 | NUCLEO 连接器 | MCU 引脚 | 外部接口 |
|---|---|---|---|
| `RS485_DE` | `P1-39` | `PD4` | `U_RS485` |
| `RS485_TX` | `P1-41` | `PD5` | `U_RS485` |
| `RS485_RX` | `P1-43` | `PD6` | `U_RS485` |
| `RS485_A` | - | - | `U_RS485/J_RS485-1` |
| `RS485_B` | - | - | `U_RS485/J_RS485-2` |
| `DI_TRIG` | `P1-23` | `PC13` | `J_DI` |
| `DO_ALARM` | `P2-48` | `PD15` | `Q_DO/J_DO` |
| `LED_ALM` | `P2-28` | `PB14` | `LED_ALM` |
| `LED_RUN` | `P1-34` | `PB0` | `LED_RUN` |
| `LED_NET` | `P1-21` | `PB7` | `LED_NET` |

### 3.2.1 MAX3485 SOP-8 引脚级连接

| MAX3485 引脚 | 引脚名 | 直接连接 |
|---|---|---|
| `1` | `RO` | `P1-43 (PD6)`，网络名 `RS485_RX` |
| `2` | `/RE` | `P1-39 (PD4)`，网络名 `RS485_DE`，与 `3-DE` 同网 |
| `3` | `DE` | `P1-39 (PD4)`，网络名 `RS485_DE`，与 `2-/RE` 同网 |
| `4` | `DI` | `P1-41 (PD5)`，网络名 `RS485_TX` |
| `5` | `GND` | `GND` |
| `6` | `A` | `J_RS485-1`，网络名 `RS485_A`，同时接 `R_TERM` 和 `D_RS485` |
| `7` | `B` | `J_RS485-2`，网络名 `RS485_B`，同时接 `R_TERM` 和 `D_RS485` |
| `8` | `VCC` | `3V3_IN`，并在 `8-VCC` 与 `5-GND` 之间放置 `C_RS485=0.1uF` |

补充说明：
- 文档中 `RS485_DE`、`RS485_TX`、`RS485_RX` 是 MCU 侧网络名；真正连到总线端子的必须是 `6-A` 和 `7-B`。
- `2-/RE` 与 `3-DE` 直接并网到 `PD4`，这样发送时关闭接收，接收时关闭发送。
- 现场设备对 `A/B`、`D+/-` 的命名可能不一致；本板内部固定按 `MAX3485` 的 `A/B` 命名，若对端标识相反，可在接线端交换两线。

### 3.3 可选扩展

| 网络名 | NUCLEO 连接器 | MCU 引脚 | 用途 |
|---|---|---|---|
| `AI_QSPI_NCS` | `P2-70` | `PG6` | `QSPI Flash` |
| `AI_QSPI_CLK` | `P2-22` | `PB2` | `QSPI Flash` |
| `AI_QSPI_IO0` | `P2-45` | `PD11` | `QSPI Flash` |
| `AI_QSPI_IO1` | `P2-43` | `PD12` | `QSPI Flash` |
| `AI_QSPI_IO2` | `P1-46` | `PE2` | `QSPI Flash` |
| `AI_QSPI_IO3` | `P2-41` | `PD13` | `QSPI Flash` |
| `GIMBAL_PWM1` | `P2-52` 或 `P2-48` | `PE9` 或 `PD15` | 云台/舵机 |
| `GIMBAL_PWM2` | `P2-56` | `PE11` | 云台/舵机 |
| `GIMBAL_PWM3` | `P2-55` | `PE13` | 云台/舵机 |

注：基础工业版中，`P2-48` 优先定义为 `DO_ALARM`；只有验证型云台版本才把该脚切回单轴舵机 PWM。

## 4. 立创EDA建库与封装建议

| 位号 | 器件建议 | 封装建议 |
|---|---|---|
| `P1` `P2` | `2x35 2.54mm Female Header` | 直插件 |
| `J_CAM` | 24P `FPC/BTB` | 按相机模组确认 |
| `U_CAM1` `U_CAM2` | `LDO` | `SOT-23-5` 或 `SOT-223` |
| `U_RS485` | `MAX3485/SN65HVD75` 类 | `SOIC-8` 或 `SOP-8` |
| `J_RS485` `J_DI` `J_DO` | 端子台 | `3.81mm` 或 `5.08mm` |
| `OK1` | 光耦输入 | `SOP-4` / `DIP-4` |
| `Q_DO` | 低边 MOSFET/NPN 驱动 | `SOT-23` |
| `J_AI` | 板对板连接器 | `1.27mm` 优先 |
| `J_SERVO1` `J_GIMBAL` | 排针/端子 | 2.54mm |

## 5. 精简 BOM（供画图与 PCB 预布局）

| 位号前缀 | 器件 | 数量 | 备注 |
|---|---|---:|---|
| `P1` | `2x35 2.54mm` 母座 | 1 | 对插 `CN11` |
| `P2` | `2x35 2.54mm` 母座 | 1 | 对插 `CN12` |
| `J_CAM` | 相机 FPC/BTB 座 | 1 | `OV5640` |
| `U_CAM1` | `3.3V -> 2.8V LDO` | 1 | 相机模拟电源 |
| `U_CAM2` | `3.3V -> 1.8V LDO` | 1 | 相机内核/IO 电源 |
| `U_RS485` | 3.3V `RS485` 收发器 | 1 | `Modbus RTU` |
| `C_RS485` | `0.1uF` | 1 | `U_RS485` 供电去耦 |
| `D_RS485` | TVS | 1 | 靠近 `J_RS485` |
| `R_TERM` | `120R` | 1 | 终端电阻，可跳线 |
| `R_BIAS_A` `R_BIAS_B` | 总线偏置电阻 | 2 | 可选预留，按系统拓扑装配 |
| `OK1` | 光耦 | 1 | `DI` 输入隔离 |
| `Q_DO` | MOSFET/NPN | 1 | `DO` 输出驱动 |
| `LED_ALM` `LED_RUN` `LED_NET` | LED | 3 | 状态指示 |
| `R_LEDx` | `1k` | 3 | LED 限流 |
| `R_SCL` `R_SDA` | `4.7k` | 2 | `SCCB` 上拉 |
| `C_DECx` | `0.1uF` | 若干 | 去耦 |
| `C_BULK` | `10uF` | 2~4 | LDO/接口缓冲 |
| `J_AI` | 板对板连接器 | 1 | 可选，默认 DNP |
| `U_QSPI` | QSPI Flash | 1 | 可选，默认 DNP |
| `J_SERVO1`/`J_GIMBAL` | 舵机接口 | 1 | 可选，默认 DNP |

## 6. 出图约束

- 基础工业版不画 `SDRAM`、`LCD`、`LTDC/FMC` 相关器件。
- 基础工业版不装云台和舵机驱动，`PD15` 固定为 `DO_ALARM`。
- `Ethernet`、`Debug UART`、`ST-LINK` 直接复用开发板，不在转接板重复实现。
- 如需转正式量产板，应以 [10_Hardware_Pinout_Productization.md](/E:/STM32/OV/docx/archive/10_Hardware_Pinout_Productization.md) 为芯片级输入重新出图。
