# 当前工作进度档案

- **当前任务目标**：将本轮对话中的硬件产品化规划、开发板转接板设计、RS485/MAX3485 接线细化、求职文案、作品集/PPT 讲稿与系统方框图等内容高密度压缩归档，形成后续可继续复用的上下文。

- **已完成步骤**：
  - 生成并完善 [AGENTS.md](/E:/STM32/OV/AGENTS.md)，整理仓库结构、构建方式、编码规范与文档索引，并整合 `CLAUDE.md` 的项目概览与 V2 架构信息。
  - 新增并扩展 [10_Hardware_Pinout_Productization.md](/E:/STM32/OV/docx/10_Hardware_Pinout_Productization.md)，明确基础工业版产品边界：不建议上 `SDRAM`、不建议上 `LCD`，保留 `Ethernet`、`RS485`、`DI/DO`、状态灯、`SWD`、`Debug UART`，并将云台/舵机降为可选扩展。
  - 在同一硬件文档中补齐芯片级量产板与 `NUCLEO-H753ZI` 开发板转接方案，输出量产级/转接板级的接口、引脚分配与 PCB/BOM 规划。
  - 新增 [11_LCEDA_Schematic_Input_NUCLEO_Adapter.md](/E:/STM32/OV/docx/11_LCEDA_Schematic_Input_NUCLEO_Adapter.md)，给出立创EDA可直接使用的原理图输入、分 sheet 要求、网络映射、器件位号和 BOM。
  - 持续细化 `RS485 + MAX3485` 设计，明确 `A/B` 差分端、`R_TERM`、偏置电阻，以及 `RO / /RE / DE / DI / VCC / GND / A / B` 的 SOP-8 逐引脚连接关系，并统一到硬件文档和立创EDA文档中。
  - 输出适合求职使用的项目经历文案，并整理为 [12_Resume_Portfolio_Guide.md](/E:/STM32/OV/docx/12_Resume_Portfolio_Guide.md)，重点统一“已完成 / 推进中 / 规划中”的面试口径。
  - 输出逐页可讲解的 [13_Portfolio_PPT_Script.md](/E:/STM32/OV/docx/13_Portfolio_PPT_Script.md)，补齐 `FreeRTOS`、`LwIP/UDP/Netconn`、`RS485/Modbus RTU`、图像链路、内存布局、产品化接口等技术页结构图和讲稿。
  - 参考半导体企业应用页的表达方式，为作品集和 PPT 新增“应用概览 / 系统优势 / 典型应用 / 产品系统方框图”等前导内容，并将方框图文案进一步压缩成适合 PPT 页面展示的正式口径。
  - 回答了“后续更换相机模组是否需要大改架构”的问题，结论是：当前工程并非只改拍摄任务即可，主要影响相机驱动、`Vision_Pipeline`、`Auto_Exposure`、`freertos.c` 与图像/内存配置层，但 `FreeRTOS`、`LwIP`、`RS485` 主体结构不必整体推翻。

- **未完成计划**：
  - `P1`：如用户需要，继续把“系统硬件方框图”细化成 PPT 图中每个框的标准命名、配色和箭头说明。
  - `P1`：如用户需要，继续输出正式答辩版 PPT 页面文案，压缩成“页面标题 + 3 条 bullet + 1 分钟口播稿”。
  - `P2`：如进入工程实施阶段，可将 `USART2 RS485`、`PD15 -> DO_ALARM`、状态灯分配等规划落实到 `.ioc` 与固件代码。
  - `P2`：如进入硬件设计阶段，可进一步输出转接板详细网络清单、连接器针脚对照和更完整的生产 BOM。

- **关键决策及原因**：
  - 基础工业版不引入 `SDRAM` 与 `LCD`：当前分辨率与功能边界下，片上资源仍可支撑；去掉两者可降低 BOM、复杂度和产品风险。
  - 基础版不保留云台/舵机为主设计功能：工业入口相机优先保证抓拍、联网与工业接口联动，云台/舵机只保留为验证型或扩展型方案。
  - `RS485` 采用 `USART2 + MAX3485`：`3.3V` 兼容、外围简单、适合与 `PLC/HMI` 做 `Modbus RTU` 对接。
  - `MAX3485` 文档必须直接使用芯片引脚名：原先仅写 `RS485_RX/TX/DE` 不够适合硬件设计，改为 `RO / /RE / DE / DI / A / B` 逐脚说明后更利于原理图和 PCB 实施。
  - 求职文案必须区分“已完成”“推进中”“规划中”：避免把 V2 重构、本地 AI 推理或量产落地说成已全部完成，保持与现有技术文档一致。
  - 作品集/PPT 增加半导体企业风格的应用介绍页：先讲应用场景、系统价值和系统框图，再进入 `FreeRTOS`、`LwIP`、`RS485` 等技术细节，更符合面试展示逻辑。
  - 产品/系统方框图定位为硬件系统级框图：用于展示模块边界和接口关系，不替代详细原理图，也不混入过多软件实现细节。

- **当前实验状态**：
  - 当前轮为文档整理与方案归档，没有运行编译、烧录或板级测试。
  - 没有训练任务、性能实验或在线测试在跑。

- **关键文件路径**：
  - 仓库与协作规范：
    - [AGENTS.md](/E:/STM32/OV/AGENTS.md)
    - [docx/07_compress_context.md](/E:/STM32/OV/docx/07_compress_context.md)
  - 硬件产品化与接口规划：
    - [docx/10_Hardware_Pinout_Productization.md](/E:/STM32/OV/docx/10_Hardware_Pinout_Productization.md)
    - [docx/11_LCEDA_Schematic_Input_NUCLEO_Adapter.md](/E:/STM32/OV/docx/11_LCEDA_Schematic_Input_NUCLEO_Adapter.md)
  - 求职与展示材料：
    - [docx/12_Resume_Portfolio_Guide.md](/E:/STM32/OV/docx/12_Resume_Portfolio_Guide.md)
    - [docx/13_Portfolio_PPT_Script.md](/E:/STM32/OV/docx/13_Portfolio_PPT_Script.md)
  - 相关技术依据：
    - [docx/08_TechnicalDesign.md](/E:/STM32/OV/docx/08_TechnicalDesign.md)
    - [docx/09_CommunicationProtocol.md](/E:/STM32/OV/docx/09_CommunicationProtocol.md)
  - 代码定位参考：
    - [APP/src/Vision_Pipeline.c](/E:/STM32/OV/APP/src/Vision_Pipeline.c)
    - [APP/src/Auto_Exposure.c](/E:/STM32/OV/APP/src/Auto_Exposure.c)
    - [APP/src/vision_capture.c](/E:/STM32/OV/APP/src/vision_capture.c)
    - [Core/Src/freertos.c](/E:/STM32/OV/Core/Src/freertos.c)
    - [APP/Inc/app_config.h](/E:/STM32/OV/APP/Inc/app_config.h)

- **当前判断与下一步**：
  - 当前对话的主要成果已经稳定沉淀为“产品化硬件规划 + 立创EDA输入 + 求职文案 + 作品集/PPT 讲稿 + 企业风格应用介绍 + 系统方框图”这一整套文档链路，前后口径基本统一。
  - 下一步最合理的方向有两类：一类是继续强化展示材料，把 PPT 图文进一步标准化；另一类是从文档转入实施，把 `RS485`、`DO_ALARM`、状态灯等规划真正落地到 `.ioc` 和代码中。
