#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "lwip/udp.h"
#include "shared_types.h"

/* Network parameters are defined in app_config.h */

typedef enum { NET_IDLE, NET_READY, NET_SENDING, NET_ERROR } NetState_t;

typedef struct {
  struct udp_pcb *upcb;
  ip_addr_t dest_addr;
  NetState_t state;
  uint32_t tx_frame_count;
} NetCtrl_t;

int8_t Net_Client_Init(void);
void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id);
void Net_Client_Diagnostic(void);

#endif /* NET_CLIENT_H */
