# Repository Guidelines

## Project Overview
`IVCIS_Edge_H753` is an industrial edge-vision firmware project built on `STM32H753ZI + OV5640 + radar`. It runs `FreeRTOS + LwIP`, uploads JPEG frames and event metadata over UDP, and accepts control commands over Ethernet or `RS485 Modbus RTU` for alarm, trigger, and PLC/HMI actions. The current product direction is a radar-triggered industrial entrance/charging-lane camera with event evidence, deterministic field I/O, and cloud/upper-computer recognition.

## Project Structure & Module Organization
`APP/Inc` and `APP/src` contain business logic. Main modules include `Vision_Pipeline`, `vision_capture`, `Motion_Detect`, `Power_Manager`, `Auto_Exposure`, `Net_Client`, `Transport_HAL`, `Alarm_Handler`, and `Servo_Control`. `APP/Test` contains embedded test entries such as `test_mode_switch.c` and `test_v2_integration.c`. `Core/Inc` and `Core/Src` are STM32CubeMX-generated startup, HAL, and FreeRTOS glue; keep custom edits inside `USER CODE BEGIN/END` blocks. `Drivers/`, `Middlewares/`, and `LWIP/` are vendor or stack code. `Debug/` and `Release/` are generated outputs and should not be treated as source.

## Architecture Notes
The target task model in `Core/Src/freertos.c` uses event-driven `Task_Camera`, queue-driven `Task_NetTx`, blocking `Task_NetRx`, framework-level `Task_Radar`, planned `Task_RS485`, and low-priority `Task_IdlePower`. `Task_Camera` waits for capture/health events, captures into a dual-buffer queue, and is notified by DCMI frame-complete thread flags. `Task_NetTx` performs UDP transmission, `Task_NetRx` processes cloud commands, `Task_Radar` triggers camera events from radar targets through `Radar_Manager`, and `Task_RS485` will expose Modbus RTU control. Ethernet and RS485 must both issue control actions through shared APIs.

Memory placement is part of the design. Do not change `D2_SRAM_SECTION`, `D1_AXI_SECTION`, or `IVCIS_ALIGN_32` casually; frame buffers, ETH DMA descriptors, and LwIP heap placement depend on them.

## Build, Test, and Development Commands
Use the checked-in scripts and existing STM32CubeIDE toolchain paths.

- `build.bat`: build the `Release` configuration with the workstation's STM32CubeIDE `make.exe`.
- `bash build_debug.sh`: build the `Debug` configuration and forward extra `make` arguments.
- `cd Release && make all`: direct Release build.
- `cd Debug && make all`: direct Debug build.

Flash and debug through STM32CubeIDE or ST-Link after a successful build.

## Coding Style & Naming Conventions
Follow the existing C style: 2-space indentation, K&R braces, uppercase macros, and module-oriented names such as `Vision_SetMode` and `NET_LINK_TIMEOUT_MS`. Keep identifiers and log output ASCII even when discussion or documentation is in Chinese. Prefer configuration in `APP/Inc/app_config.h` and debug switches in `APP/Inc/debug_config.h`. Reuse `DBG_INFO`, `DBG_WARN`, `DBG_ERROR`, `DBG_NET`, and `DBG_VISION` instead of raw `printf`.

Keep modules focused and loosely coupled. Avoid unnecessary heap use, extra intermediate buffers, and oversized source files when logic can be split cleanly.

## Testing Guidelines
This project uses embedded test flows, not host-side unit tests. Add new test files under `APP/Test` using the `test_<module>.c` pattern. Select the active scenario via `TEST_SELECT` in `APP/Inc/debug_config.h`, rebuild, flash, and verify behavior through UART3 logs at `115200`. Keep test-only logic behind the test selector.

For networking work, prefer the `Netconn` API. Do not call the LwIP Raw API directly outside `tcpip_thread`.

## Commit & Pull Request Guidelines
Recent history mixes short English prefixes such as `feat:` with concise Chinese summaries. Keep commit subjects short, imperative, and scoped to one change, for example `feat: refine UDP frame queue` or `修复 JPEG 缓冲切换`. PRs should state the hardware/setup used, summarize functional impact, list touched modules, and include serial logs or screenshots when behavior changes. Explicitly call out changes to `.ioc`, linker scripts, generated CubeMX files, or local toolchain paths.

## Configuration & Safety Notes
Treat `APP/Inc/app_config.h` as the source for camera resolution, IP/port settings, exposure thresholds, queue depth, and memory-section attributes. Use `APP/Inc/debug_config.h` for debug and test switches rather than hardcoding diagnostics in module code. Do not commit secrets, accidental device-specific IP changes, or machine-local paths unless the change is deliberate for the team.

## Documentation
Read `docx/README.md` before changing architecture or protocols. Current implementation references:

- `docx/01_Charter.md`: project constraints and guardrails
- `docx/02_Requirements.md`: current functional requirements
- `docx/03_Integrated_Project_Design.md`: primary consolidated design document covering background, architecture, software, hardware, protocol, scenarios, event evidence, Edge AI future work, and radar flow
- `docx/04_FileList.md`: code module index
- `docx/05_TaskBreakdown.md`: implementation tasks and milestones
- `docx/06_Code Style.md`: local coding style notes
- `docx/11_LCEDA_Schematic_Input_NUCLEO_Adapter.md`: JLCEDA schematic input for the NUCLEO-H753ZI adapter board
- `docx/12_Resume_Portfolio_Guide.md`: resume-ready project wording and portfolio/PPT talking points
- `docx/13_Portfolio_PPT_Script.md`: slide-by-slide portfolio/PPT script with source references and architecture diagrams

Historical/detail source docs absorbed by `03_Integrated_Project_Design.md` are under `docx/archive/`. Reference materials are under `docx/reference/`; process notes are under `docx/work_notes/`.
