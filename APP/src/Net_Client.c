/**
 * @file    Net_Client.c
 * @brief   UDP client with IVCIS chunk headers (zero-copy)
 */

#include "Net_Client.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "lwip/etharp.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "stm32h7xx_hal.h"

/* P4: private control block */
typedef struct {
  struct udp_pcb *upcb;
  ip_addr_t dest_addr;
  NetState_t state;
  uint32_t tx_frame_count;
} NetCtrl_t;

static NetCtrl_t s_net = {0};

/* P5: read-only diagnostic (extern ETH handle) */
extern ETH_HandleTypeDef heth;

/* ---- Public API ---- */

int8_t Net_Client_Init(void) {
  s_net.upcb = udp_new();
  if (s_net.upcb == NULL) {
    s_net.state = NET_ERROR;
    DBG_NET("UDP PCB alloc FAIL");
    return -1;
  }

  IP4_ADDR(&s_net.dest_addr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2,
           DEST_IP_ADDR3);

  /* P6: check udp_bind return */
  err_t bind_err = udp_bind(s_net.upcb, IP_ADDR_ANY, UDP_LOCAL_PORT);
  if (bind_err != ERR_OK) {
    DBG_ERROR("[Net] udp_bind fail: %d", (int)bind_err);
    udp_remove(s_net.upcb);
    s_net.upcb = NULL;
    s_net.state = NET_ERROR;
    return -1;
  }

  /* Pre-resolve destination MAC via ARP */
  struct netif *nif = netif_default;
  if (nif != NULL) {
    etharp_request(nif, &s_net.dest_addr);
  }

  s_net.state = NET_READY;
  DBG_NET("Init OK, port=%d -> %d.%d.%d.%d:%d", UDP_LOCAL_PORT, DEST_IP_ADDR0,
          DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3, UDP_REMOTE_PORT);
  return 0;
}

void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id) {
  if ((s_net.state != NET_READY) || (pData == NULL) || (len == 0U)) {
    return;
  }

  /* P3: ensure DCache coherency before DMA access */
  SCB_CleanDCache_by_Addr((uint32_t *)((uint32_t)pData & ~31U),
                          (int32_t)(len + 31U));

  /* P1: compute chunk count */
  uint32_t payload_cap = NET_MAX_UDP_PAYLOAD - (uint32_t)sizeof(NetChunkHdr_t);
  uint16_t chunk_cnt = (uint16_t)((len + payload_cap - 1U) / payload_cap);

  uint32_t offset = 0U;
  uint32_t fail_streak = 0U;
  s_net.state = NET_SENDING;

  for (uint16_t ci = 0U; ci < chunk_cnt; ci++) {
    uint32_t chunk_len = len - offset;
    if (chunk_len > payload_cap) {
      chunk_len = payload_cap;
    }

    /* Allocate header pbuf (RAM) */
    struct pbuf *hdr_p =
        pbuf_alloc(PBUF_TRANSPORT, (uint16_t)sizeof(NetChunkHdr_t), PBUF_RAM);
    if (hdr_p == NULL) {
      fail_streak++;
      break;
    }

    /* P1: fill IVCIS chunk header */
    NetChunkHdr_t *hdr = (NetChunkHdr_t *)hdr_p->payload;
    hdr->magic = IVCIS_MAGIC;
    hdr->frame_id = frame_id;
    hdr->chunk_idx = ci;
    hdr->chunk_cnt = chunk_cnt;
    hdr->total_size = len;
    hdr->flags = 0U;
    hdr->reserved[0] = 0U;
    hdr->reserved[1] = 0U;
    hdr->reserved[2] = 0U;

    /* Data pbuf (ROM — zero-copy) */
    struct pbuf *dat_p = pbuf_alloc(PBUF_RAW, (uint16_t)chunk_len, PBUF_ROM);
    if (dat_p == NULL) {
      pbuf_free(hdr_p);
      fail_streak++;
      break;
    }
    dat_p->payload = (void *)(pData + offset);

    pbuf_chain(hdr_p, dat_p);

    err_t err =
        udp_sendto(s_net.upcb, hdr_p, &s_net.dest_addr, UDP_REMOTE_PORT);
    pbuf_free(hdr_p);

    if (err == ERR_OK) {
      offset += chunk_len;
      fail_streak = 0U;
    } else {
      fail_streak++;
    }

    /* P2: drop frame after 3 consecutive failures */
    if (fail_streak >= 3U) {
      DBG_WARN("[Net] Frame #%lu dropped (3 fails)", frame_id);
      break;
    }

    /* Yield every 10 chunks to avoid bus starvation */
    if (((ci + 1U) % 10U) == 0U) {
      osDelay(1);
    }
  }

  s_net.tx_frame_count++;
  s_net.state = NET_READY;
  DBG_NET("F#%lu: %lu/%luB sent (%lu chunks)", frame_id, offset, len,
          (uint32_t)chunk_cnt);
}

void Net_Client_Diagnostic(void) {
  /* P5: read-only reporting, no register writes */
  DBG_NET("=== HW Diag ===");
  DBG_NET("ETH state:0x%lX  TX_desc[0]:0x%lX  RX_desc[0]:0x%lX",
          (uint32_t)heth.gState, heth.TxDescList.TxDesc[0],
          heth.RxDescList.RxDesc[0]);
  DBG_NET("Frames sent: %lu", s_net.tx_frame_count);

  if (heth.TxDescList.TxDesc[0] < 0x30000000U) {
    DBG_ERROR("[Net] TX desc NOT in D2 SRAM!");
  }
}

NetState_t Net_Client_GetState(void) { return s_net.state; }
uint32_t Net_Client_GetTxCount(void) { return s_net.tx_frame_count; }
