#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "shared_types.h"

typedef enum {
  NET_IDLE = 0,
  NET_READY,
  NET_SENDING,
  NET_ERROR
} NetState_t;

int8_t Net_Client_Init(void);
int8_t Net_Client_InitRx(void);

void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id);
bool Net_Client_RecvCommand(IVCIS_Command_t *cmd);

void Net_Client_Diagnostic(void);
NetState_t Net_Client_GetState(void);
NetState_t Net_Client_GetTxState(void);
bool Net_Client_IsRxReady(void);
uint32_t Net_Client_GetTxCount(void);

#endif /* NET_CLIENT_H */
