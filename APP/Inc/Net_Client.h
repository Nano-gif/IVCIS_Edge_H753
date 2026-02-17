#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "shared_types.h"

/* 网络状态 (不透明对象 — 内部结构隐藏在 .c 中) */
typedef enum { NET_IDLE, NET_READY, NET_SENDING, NET_ERROR } NetState_t;

/** 初始化 UDP 客户端，成功返回 0 */
int8_t Net_Client_Init(void);

/** 发送图像数据 (Raw UDP 分片模式，零拷贝) */
void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id);

/** 只读 ETH 硬件诊断转储 */
void Net_Client_Diagnostic(void);

/** 获取当前网络状态 */
NetState_t Net_Client_GetState(void);

/** 获取已发送的总帧数 */
uint32_t Net_Client_GetTxCount(void);

/** 轮询收到的云端指令。如果有新指令返回 true。
 */
bool Net_Client_RecvCommand(IVCIS_Command_t *cmd);

#endif /* NET_CLIENT_H */
