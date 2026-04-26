# IVCIS_Edge_H753

`IVCIS_Edge_H753` 是基于 `STM32H753ZI + OV5640 + radar` 的工业边缘视觉相机固件项目。当前定位是高速入口、收费/充电车道、园区通行口等场景中的“雷达触发 + 图像取证 + 工业联动”设备。

设备侧运行 `FreeRTOS + LwIP`，负责雷达目标触发、相机采集、JPEG 帧上传、状态管理、`RS485 Modbus RTU`、DI/DO 与报警/云台扩展控制。复杂车牌/车型识别和完整业务决策由上位机或云端完成。

## Current Scope

- `OV5640` 采集 JPEG/灰度图像，使用双缓冲和事件驱动任务降低阻塞。
- 雷达目标用于触发抓拍并提升雨雾、弱光等场景下的触发可靠性。
- 以太网通过 UDP 上传 JPEG 帧和事件数据。
- `RS485 Modbus RTU` 面向 PLC/HMI，提供状态读取、参数配置、触发抓拍和报警控制。
- 基础版不内置 LCD、SDRAM 或本地复杂 AI；Edge AI、云台和更多外设作为扩展方向。

## Hardware Baseline

验证形态为 `NUCLEO-H753ZI + 扩展 PCB`。开发板保留以太网 PHY、RJ45、ST-LINK 和 USB 调试，扩展板补齐 Camera、Radar、RS485、DI/DO、状态灯和保护电路。

### RS485 / MAX3485

当前固件按 `USART2 + MAX3485` 实现半双工 RS485：

| MAX3485 引脚 | 连接 | 说明 |
| --- | --- | --- |
| `RO` | `PD6 / USART2_RX` | MCU 接收 |
| `/RE` | `PD4 / RS485_DE` | 与 `DE` 同网 |
| `DE` | `PD4 / RS485_DE` | 高电平发送，低电平接收 |
| `DI` | `PD5 / USART2_TX` | MCU 发送 |
| `A` | `J_RS485-1` | RS485 总线 A |
| `B` | `J_RS485-2` | RS485 总线 B |
| `VCC/GND` | `3V3/GND` | 就近放置 `0.1uF` 去耦 |

`PD5/PD6/PD4` 是 MCU 侧信号，不直接等同于总线端子的 `A/B`。终端电阻、TVS 和可选偏置电阻应布置在 `A/B` 总线侧。

## Software Architecture

主要任务位于 `Core/Src/freertos.c`：

- `Task_Camera`: 事件驱动采集，维护双缓冲并向帧队列投递。
- `Task_NetTx`: 消费帧队列，通过 UDP 上传图像。
- `Task_NetRx`: 阻塞接收云端/上位机控制命令。
- `Task_Radar`: 接收雷达目标并触发相机抓拍。
- `Task_RS485`: 处理 Modbus RTU 帧，响应 PLC/HMI。
- `Task_Control`: 统一执行 Ethernet 与 RS485 下发的报警/舵机控制。
- `Task_IdlePower`: 低优先级空闲省电任务。

关键应用模块在 `APP/Inc` 和 `APP/src`：

- `Vision_Pipeline`, `vision_capture`, `Motion_Detect`, `Auto_Exposure`
- `Radar_Manager`
- `Net_Client`, `Transport_HAL`, `Transport_ETH`
- `RS485_Driver`, `RS485_Manager`, `Modbus_RTU`, `Modbus_RegMap`
- `Control_Manager`, `Alarm_Handler`, `Servo_Control`

## RS485 Driver Flow

`RS485_Driver` 完成硬件层工作：

1. 初始化 `USART2`，波特率默认 `115200 8N1`。
2. 初始化 `PD4` 为 MAX3485 方向控制 GPIO。
3. 接收方向下启动 UART 中断接收，收到字节后写入 `RS485_Manager` 的 RX Stream Buffer。
4. `Task_RS485` 按 Modbus RTU 帧长和帧间隔组帧。
5. 协议层生成响应后写入 TX Stream Buffer。
6. 驱动层拉高 `PD4` 发送响应，发送完成后拉低 `PD4` 回到接收态。

## Build And Test

使用仓库内脚本和 STM32CubeIDE 工具链路径：

```bat
build.bat
```

构建 `Release` 配置。当前 `Release` 规则按生产模式编译，`TEST_SELECT=0`。

```bash
bash build_debug.sh
```

构建 `Debug` 配置。当前默认 `TEST_SELECT=6`，用于运行 V2 集成测试入口。

也可以直接进入构建目录：

```bash
cd Debug && make all
cd Release && make all
```

烧录和调试通过 STM32CubeIDE 或 ST-LINK 完成。串口日志默认走 `USART3`，波特率由当前工程配置决定。

## Modbus RTU Support

已实现的功能码：

- `0x02`: Read Discrete Inputs
- `0x03`: Read Holding Registers
- `0x06`: Write Single Register
- `0x10`: Write Multiple Registers

寄存器表覆盖设备信息、功耗状态、网络状态、雷达状态、抓拍触发、报警控制、雷达使能和基础配置。详细定义见 `docx/03_Integrated_Project_Design.md` 与 `docx/archive/09_CommunicationProtocol.md`。

## Repository Layout

```text
APP/Inc              Application headers
APP/src              Application modules
APP/Test             Embedded test flows
Core/Inc             STM32 HAL/Cube headers
Core/Src             Startup glue, peripheral init, FreeRTOS tasks
Drivers/             STM32 HAL and BSP drivers
LWIP/                LwIP app and target glue
Middlewares/         FreeRTOS and LwIP middleware
docx/                Requirements, design docs, hardware docs, portfolio docs
Debug/ Release/      Generated build outputs and make rules
```

## Documentation Index

- `AGENTS.md`: contributor and agent guide.
- `docx/README.md`: documentation map.
- `docx/03_Integrated_Project_Design.md`: consolidated architecture, software, hardware, protocol and scenario design.
- `docx/11_LCEDA_Schematic_Input_NUCLEO_Adapter.md`: NUCLEO adapter board schematic input.
- `docx/12_Resume_Portfolio_Guide.md`: resume and portfolio wording.
- `docx/13_Portfolio_PPT_Script.md`: PPT slide-by-slide script.

Historical source documents are under `docx/archive/`.

## Notes

- Keep custom edits in Cube-generated files inside `USER CODE BEGIN/END` where possible.
- Do not use LwIP Raw API directly outside `tcpip_thread`; application code should use Netconn or project wrappers.
- Keep Ethernet and RS485 control actions routed through `Control_Manager` so PLC/HMI and cloud commands share one execution path.
- Do not commit machine-local secrets or accidental IP changes.
