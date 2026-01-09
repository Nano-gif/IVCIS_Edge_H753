### IVCIS_Edge_H753 工程蓝图 (V6.0 联调版)

IVCIS_Edge_H753/
├── Core/
│   ├── Src/
│   │   ├── main.c                  // [✔] 修正 MPU TEX Level 1 (Normal Memory)
│   │   │                           // [✔] 修正堆栈位置至 RAM_D1，解决 sbrk 冲突
│   │   ├── freertos.c              // [✔] 任务架构：Camera(High), Net(Normal)
│   │   │                           // [✔] LwIP 异步初始化流 (MX_LWIP_Init -> Net_Client_Init)
│   │   ├── ethernetif.c            // [✔] 核心修正：描述符与池强制对齐 32 字节并归位 D2
│   │   └── stm32h7xx_it.c          // [✔] 中断向量接管 (SVC/PendSV/SysTick)
│   └── Inc/ ...
│
├── App/ (用户应用层)
│   ├── Inc/
│   │   ├── app_config.h            // [✔] 定义 800x480 分辨率、静态IP及内存段宏
│   │   ├── shared_types.h          // [✔] 定义 VisionState_t 及状态机
│   │   ├── Vision_Pipeline.h       // [✔] 视觉管线接口 (支持全帧流式压缩)
│   │   └── Net_Client.h            // [✔] 网络接口 (支持 UDP 零拷贝发送与硬件诊断)
│   │
│   ├── Src/
│   │   ├── Vision_Pipeline.c       // [✔] 实现 DCMI 条带化采集 (D2域)
│   │   │                           // [✔] 实现 JPEG 硬件流式接力压缩 (D1域)
│   │   │                           // [✔] 物理层补丁：0x302D 驱动增强与采样相位调整
│   │   │
│   │   ├── Net_Client.c            // [🚧] UDP 传输核心：实现 PBUF_ROM 零拷贝分片
│   │   │                           // [🚧] 硬件诊断：实时监控 ETH-DMA 寄存器状态
│   │   │
│   │   └── AI_Inference.c          // [TODO] 等待网络跑通后接入 X-CUBE-AI
│
├── Drivers/
│   └── BSP/
│       ├── ov5640/                 // [✔] 正点原子驱动适配版 (已注入降速与物理补丁)
│       └── lan8742.c               // [✔] 确认 PHY 地址为 0
│
├── Middlewares/
│   ├── Third_Party/LwIP/           // [✔] 已配置软件校验和 (Checksum Disable)
│   └── ST/AI/                      // [TODO] 待启用
│
└── LinkerScript.ld                  // [✔] 终极修正：物理隔离 D1/D2，补齐 _end 锚点
```

---

### 当前阶段总结与下一步 (Action Plan)

#### 1. 硬件层 (Hardware Layer)
*   **状态**：DCMI 采样趋于稳定，PHY 物理链路已通（0x782D）。
*   **痛点**：杜邦线引入的相位偏移仍导致图像条纹，已通过软件相位反转（RISING）对冲。
*   **行动**：维持现状，先攻克数据传输。

#### 2. 视觉层 (Vision Layer)
*   **状态**：**[重大突破]** 已实现 800x480 全帧 JPEG 编码，支持 `GetDataCallback` 接力。
*   **行动**：已准备好输出 `FF D8 ... FF D9` 的完整合法文件。

#### 3. 网络层 (Network Layer)
*   **状态**：**[当前攻坚点]** 软件逻辑已闭环（UDP Client 就绪），但硬件 DMA 存在“基地址漂移”。
*   **行动**：通过 `Net_Client_Diagnostic` 读取硬件寄存器 `ETH->DMACTDR`，确认地址是否真正进入 D2 域。若寄存器仍报错，将执行“寄存器级强制重定向”。

#### 4. 算法层 (AI Layer)
*   **状态**：暂缓。
*   **计划**：在网络上报第一张图片成功后，立即在 `Vision_Pipeline` 插入 DMA2D 缩放逻辑，开始 TinyML 联调。

---