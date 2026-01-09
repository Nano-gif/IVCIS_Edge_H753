#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include <stdint.h>
#include "lwip/udp.h"

typedef enum {
    NET_IDLE = 0,
    NET_WAIT_LINK,
    NET_READY,
    NET_SENDING,
    NET_ERROR
} NetState_t;

typedef struct {
    struct udp_pcb *upcb;
    ip_addr_t dest_addr;
    NetState_t state;
    uint32_t tx_frame_count;
    uint32_t last_error_code;
} NetCtrl_t;

int8_t Net_Client_Init(void);
void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id);
void Net_Client_Diagnostic(void);

extern NetCtrl_t g_net_ctrl;

#endif
