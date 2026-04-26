# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

IVCIS_Edge_H753 — 工业智能相机边缘计算平台。基于 STM32H753ZI + OV5640，运行 FreeRTOS + LwIP，通过以太网 UDP 零拷贝上传 JPEG 至云端，通过 RS485 Modbus RTU 与 PLC/HMI 交互，接收云端指令执行舵机/报警动作。

**当前状态**: V1 基础功能已完成，正在进行 V2 架构重构（并行化 + Netconn 迁移 + Transport 抽象层 + RS485）。

## Build

使用 STM32CubeIDE 工具链，Windows 环境：
```bash
# 命令行构建 (Release 配置)
./build.bat
# 或直接调用 make:
cd Release && make all
```
Make 路径硬编码在 `build.bat` 中，指向 `E:\Program Files\STM32CubeIDE_2.0.0\...\make.exe`。

烧录：通过 STM32CubeIDE 或 ST-Link 烧录至 NUCLEO-H753ZI。

## Architecture

### V2 目标: 四任务 FreeRTOS 模型 (Core/Src/freertos.c)
- **Task_Camera** (High, 8KB): 生产者 — 双缓冲采集 + 质量控制 + 功耗管理，帧入队后立即采下一帧
- **Task_NetTx** (AboveNormal, 8KB): 消费者 — 从帧队列取数据，Netconn UDP 零拷贝发送
- **Task_NetRx** (Normal, 4KB): Netconn UDP 阻塞接收，分发云端控制指令
- **Task_RS485** (Normal, 2KB): Modbus RTU 从站，寄存器读写，PLC/HMI 控制

> **双控制源**: Task_NetRx 和 Task_RS485 均可触发 Alarm/Servo/PowerMode 等执行动作，共用同一组 API，部署时可二选一或并存 (Last-Write-Wins)

### V1 当前: 双任务模型 (待重构)
- **Task_Camera**: 采集 + 发送串行执行（Net_Client_SendImage 在 Camera 任务内同步调用）
- **Task_Net**: 仅做指令接收轮询

### APP 层模块 (APP/src/, APP/Inc/)
所有业务逻辑在 APP 层，Core 层仅做 HAL/外设初始化：

| 模块 | 职责 |
|---|---|
| Vision_Pipeline | OV5640 DCMI 采集，JPEG/灰度模式切换，双缓冲管理 |
| vision_capture | 底层 DCMI DMA 控制 (ForceStop/CaptureOne) |
| Motion_Detect | 帧差法运动检测 (灰度模式，纯整数) |
| Power_Manager | 三级功耗状态机: FULL → LIGHT → DETECT |
| Auto_Exposure | DETECT 模式下 SCCB 步进调节曝光/增益 |
| Net_Client | UDP 发送/接收 (Netconn API, 零拷贝, TX/RX 独立连接) |
| Transport_HAL | V2 新增: 可插拔通信通道抽象 (ETH/RS485) |
| Modbus_Slave | V2 新增: RS485 Modbus RTU 从站 |
| Alarm_Handler | 云端违规指令 → PB14 LED 报警 |
| Servo_Control | 云端舵机指令 → PD15 TIM4_CH4 PWM (0°-180°) |

### 关键内存布局 (D2 SRAM)
- Frame Buffer A (100KB) + Frame Buffer B (100KB): DCMI DMA 双缓冲
- ETH DMA 描述符 + LwIP Heap (~40KB)
- 灰度帧缓冲 (19.2KB)
- 宏: `D2_SRAM_SECTION` = `__attribute__((section(".RamDataSection")))`, `IVCIS_ALIGN_32`

### 协议
- **上行 (MCU→Cloud)**: UDP 分片，NetChunkHdr_t 包头，magic=`0x49564349` ("IVCI")，1400B/片
- **下行 (Cloud→MCU)**: IVCIS_Command_t，magic=`0x49564352` ("IVCR")，cmd_type: 0x01=报警, 0x02=舵机
- **RS485**: Modbus RTU 从站，寄存器表详见 `docx/09_CommunicationProtocol.md`

### 共享类型 (APP/Inc/shared_types.h)
所有模块间传递的数据结构: VisionMode_t, PowerMode_t, FrameDesc_t, NetChunkHdr_t, IVCIS_Command_t, ImageQuality_t。

## Configuration

- **app_config.h**: 所有可调参数集中于此 — 摄像头分辨率、网络 IP/端口、功耗切换时间、曝光阈值、内存段宏
- **debug_config.h**: 调试开关 — `DEBUG_ENABLE` 总开关, `TEST_SELECT` (0=生产, 1-5=各模块测试), 模块级开关 (DEBUG_NET, DEBUG_VISION 等)
- 调试输出通过 `dbg_printf()` (nanoprintf + UART3 115200)，宏: `DBG_INFO/DBG_WARN/DBG_ERROR/DBG_NET/DBG_VISION`

## Code Conventions

- **语言**: 对话使用中文，代码标识符和日志使用英文 ASCII
- **100 行原则**: 单一 .c 文件业务逻辑不超 100 行（注释除外），超限必须拆分模块
- **原子化架构**: 主文件仅引用和组装子模块，不写业务逻辑；高内聚低耦合
- **嵌入式优化**: 复用现有变量，避免不必要的内存分配，不引入多余中间变量
- **注释**: 简明扼要紧贴代码，修改说明放代码外讲解
- **日志格式**: `[DEBUG|INFO|WARN|ERROR] [ModuleName] Message`
- **CubeMX 兼容**: Core 层文件中的修改必须放在 `USER CODE BEGIN/END` 块内，否则重新生成会覆盖
- **TDD 流程**: 通过 `debug_config.h` 中 `TEST_SELECT` 切换测试模式，测试文件在 APP/src/test_*.c
- **LwIP API**: V2 使用 Netconn API (线程安全)，禁止在非 tcpip_thread 中直接调用 Raw API

## Documentation

`/docx` 目录包含文档体系:
- `01_Charter.md` — 项目宪章与核心约束
- `02_Requirements.md` — 功能需求定义 (含 V2 新增需求)
- `03_Architecture.md` — 技术架构、内存布局、数据流
- `05_TaskBreakdown.md` — TDD 驱动的任务清单与里程碑 (V1 已完成, V2 进行中)
- `08_TechnicalDesign.md` — 技术设计文档 (V1 分析 + V2 目标设计 + 重构路线)
- `09_CommunicationProtocol.md` — 通信协议规格 (UDP 帧格式、Modbus 寄存器表、消息序列图)
