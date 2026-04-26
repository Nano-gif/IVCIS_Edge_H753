# IVCIS 文件与模块索引

本文档只记录当前有效的代码结构。历史架构和旧设计细节见 `docx/archive/`。

## 目录结构

```text
E:/STM32/OV
├── APP/Inc                 # 应用层头文件
├── APP/src                 # 应用层实现
├── APP/Test                # 嵌入式测试入口
├── Core/Inc, Core/Src      # CubeMX/HAL/FreeRTOS glue
├── Drivers                 # STM32 HAL、BSP、OV5640、LAN8742
├── LWIP, Middlewares       # LwIP、FreeRTOS 等中间件
├── docx                   # 当前文档、归档文档和参考资料
├── Debug, Release          # 生成输出和 Makefile 片段
└── IVCIS_Edge_H753.ioc     # CubeMX 配置
```

## 当前核心模块

| 模块 | 主要文件 | 职责 |
|---|---|---|
| Camera task API | `APP/Inc/Camera_Task.h`, `Core/Src/freertos.c` | 相机事件唤醒、健康抓拍请求、DCMI 帧完成通知 |
| Vision pipeline | `APP/src/Vision_Pipeline.c`, `APP/src/vision_capture.c` | OV5640 初始化、JPEG/Gray 模式、DCMI+DMA 采集、双缓冲管理 |
| Radar manager | `APP/src/Radar_Manager.c` | 解析/保存雷达目标摘要, 判断抓拍区并触发相机事件 |
| Network client | `APP/src/Net_Client.c` | Netconn UDP 图传、云端指令阻塞接收 |
| Transport abstraction | `APP/src/Transport_HAL.c`, `APP/src/Transport_ETH.c` | 预留 ETH/RS485 等可插拔通信通道 |
| Power manager | `APP/src/Power_Manager.c` | 当前功耗状态与低频健康抓拍节奏 |
| Auto exposure | `APP/src/Auto_Exposure.c` | 图像亮度、清晰度评估和曝光调节 |
| Alarm/DO | `APP/src/Alarm_Handler.c` | 报警状态、外部 DO 联动 |
| Servo legacy | `APP/src/Servo_Control.c` | 历史舵机接口, 当前产品主设计不依赖 |

## FreeRTOS 任务

| 任务 | 文件 | 当前状态 |
|---|---|---|
| `Task_Camera` | `Core/Src/freertos.c` | 已接入事件驱动 + 低频健康超时 |
| `Task_NetTx` | `Core/Src/freertos.c` | 已通过帧队列发送 JPEG |
| `Task_NetRx` | `Core/Src/freertos.c` | 已通过 Netconn 阻塞接收云端命令 |
| `Task_Radar` | `Core/Src/freertos.c`, `APP/src/Radar_Manager.c` | 已接入策略框架, 待替换真实雷达驱动 |
| `Task_RS485` | 规划中 | 待实现 UART/IDLE/DMA + Modbus RTU |
| `Task_IdlePower` | `Core/Src/freertos.c` | 已作为低优先级空闲低功耗任务 |

## 测试入口

| 文件 | 用途 |
|---|---|
| `APP/Test/test_v2_integration.c` | 双缓冲、帧队列、Netconn、相机事件、雷达触发策略集成测试 |
| `APP/Test/test_net_client.c` | 网络客户端测试 |
| `APP/Test/test_net_diag.c` | 以太网/LwIP 诊断 |
| `APP/Test/test_power_manager.c` | 功耗状态测试 |

通过 `APP/Inc/debug_config.h` 中的 `TEST_SELECT` 选择测试场景。

## 文档结构

| 路径 | 内容 |
|---|---|
| `docx/README.md` | 文档索引 |
| `docx/03_Integrated_Project_Design.md` | 当前主设计文档 |
| `docx/archive/` | 已被主设计吸收的历史文档 |
| `docx/reference/` | 数据手册、图片、格式模板 |
| `docx/work_notes/` | 过程记录和上下文压缩 |
