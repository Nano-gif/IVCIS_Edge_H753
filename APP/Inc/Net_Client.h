#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "shared_types.h"

/* Net state (opaque — internal struct hidden in .c) */
typedef enum { NET_IDLE, NET_READY, NET_SENDING, NET_ERROR } NetState_t;

/** Initialize UDP client, returns 0 on success */
int8_t Net_Client_Init(void);

/** Send image with IVCIS chunk headers (zero-copy) */
void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id);

/** Read-only ETH hardware diagnostic dump */
void Net_Client_Diagnostic(void);

/** Get current net state */
NetState_t Net_Client_GetState(void);

/** Get total frames sent */
uint32_t Net_Client_GetTxCount(void);

#endif /* NET_CLIENT_H */
