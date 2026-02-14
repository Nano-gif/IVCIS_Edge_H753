# IVCIS 高速入口智能视觉终端 - 文件列表

## 项目目录结构

```
e:/STM32/OV/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32h7xx_hal_conf.h
│   │   └── stm32h7xx_it.h
│   └── Src/
│       ├── main.c                    # 系统入口，仅组装子模块 (编排者)
│       ├── stm32h7xx_it.c            # 中断服务 (ISR短小化)
│       ├── dcmi.c                    # DCMI 外设初始化
│       └── gpio.c                    # GPIO 配置
│
├── App/                              # 应用层模块 (核心业务)
│   ├── Inc/
│   │   ├── app_config.h              # 全局配置参数 (const LUT)
│   │   ├── debug_config.h            # 日志宏定义
│   │   ├── shared_types.h            # 共享类型 (stdint.h, 结构体对齐)
│   │   ├── Vision_Pipeline.h         # 视觉采集接口 (Opaque Pointer)
│   │   ├── Motion_Detect.h           # 帧差法检测接口
│   │   ├── Net_Client.h              # 网络上报/接收结果接口
│   │   ├── Alarm_Handler.h           # 报警控制接口
│   │   ├── Power_Manager.h           # 电源管理接口
│   │   ├── Auto_Exposure.h           # 自适应曝光 + 质量评估接口
│   │   └── Servo_Control.h           # 舵机 PWM 控制接口
│   └── Src/
│       ├── Vision_Pipeline.c         # DCMI + OV5640 JPEG/灰度模式切换
│       ├── Motion_Detect.c           # 帧差法运动检测 (纯数学)
│       ├── Net_Client.c              # LwIP UDP 发送 + 接收云端结果
│       ├── Alarm_Handler.c           # GPIO 声光报警
│       ├── Power_Manager.c           # 三级功耗模式状态机
│       ├── Auto_Exposure.c           # SCCB 参数调整 + CMSIS-DSP 评估
│       └── Servo_Control.c           # TIM4 PWM 舵机驱动
│
├── Drivers/
│   ├── BSP/
│   │   ├── ov5640/                   # OV5640 摄像头驱动
│   │   │   ├── ov5640.c
│   │   │   ├── ov5640.h
│   │   │   └── sccb.c                # SCCB (I2C) 通信协议
│   │   └── Components/
│   │       └── lan8742/              # LAN8742 PHY 驱动
│   └── STM32H7xx_HAL_Driver/         # STM32 HAL 库
│
├── LWIP/
│   ├── App/
│   │   └── lwip.c                    # LwIP 初始化
│   └── Target/
│       ├── ethernetif.c              # 以太网接口适配
│       └── lwipopts.h                # LwIP 配置
│
├── Middlewares/
│   └── Third_Party/
│       ├── FreeRTOS/                 # FreeRTOS 内核
│       └── LwIP/                     # LwIP 协议栈
│
├── docx/                             # 项目文档
│   ├── 01_Charter.md                 # 宪章
│   ├── 02_Requirements.md            # 需求
│   ├── 03_Architecture.md            # 架构
│   ├── 04_FileList.md                # 文件列表 (本文件)
│   ├── 05_TaskBreakdown.md           # 任务分解
│   └── coding_guidance.md            # 编码规范 (Barr Group + ARM 优化)
│
├── IVCIS_Edge_H753.ioc               # STM32CubeMX 配置文件
├── STM32H753ZITX_FLASH.ld            # 链接脚本
└── Makefile                          # 构建脚本
```

## 模块文件清单

### 核心应用模块

| 文件 | 行数限制 | 职责 | 依赖 | 编码规范要点 |
|------|----------|------|------|-------------|
| `Vision_Pipeline.c` | ≤80 | OV5640 JPEG/灰度模式切换、DMA 采集 | DCMI HAL, OV5640 BSP | Opaque Pointer, volatile DMA buf |
| `Motion_Detect.c` | ≤50 | 帧差法运动检测 (纯整数运算) | 无外部依赖 | static 内部函数, restrict 指针 |
| `Net_Client.c` | ≤80 | UDP 零拷贝发送 + 接收云端结果 | LwIP (Raw API) | PBUF_ROM, NULL check, 无 malloc |
| `Alarm_Handler.c` | ≤50 | GPIO 控制、定时报警 | HAL_GPIO, TIM | ISR 短小化, deferred processing |
| `Power_Manager.c` | ≤80 | 三级功耗状态机 | HAL_RCC, Motion_Detect | 事件驱动状态机 |
| `Auto_Exposure.c` | ≤80 | 亮度/模糊度评估 + SCCB 调参 | BSP/ov5640, CMSIS-DSP | FPU 用 f 后缀, arm_var_f32 |
| `Servo_Control.c` | ≤50 | TIM4 PWM 舵机驱动 | HAL_TIM | 静态分配, 角度范围校验 |

### 配置文件

| 文件 | 职责 | 编码规范要点 |
|------|------|-------------|
| `app_config.h` | 网络参数、缓冲区大小、阈值定义 | const LUT 放 Flash |
| `debug_config.h` | 日志开关、模块使能 | 条件编译宏 |
| `shared_types.h` | 跨模块共享的结构体和枚举 | stdint.h only, 结构体按大小降序排列 |
| `lwipopts.h` | LwIP 内存池、校验和、功能开关 | 静态内存池 |
| `stm32h7xx_hal_conf.h` | HAL 模块使能、ETH 描述符数量 | - |
| `coding_guidance.md` | 编码规范文档 | - |

---

**文档版本**: v2.0  
**更新日期**: 2026-02-12  
**维护者**: IVCIS Team
