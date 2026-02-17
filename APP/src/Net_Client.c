/**
 * @file    Net_Client.c
 * @brief   UDP 核心传输与接收 (零拷贝 + IVCIS 协议)
 */

#include "Net_Client.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "stm32h7xx_hal.h"
#include <string.h>

typedef struct {
  struct udp_pcb *upcb;
  ip_addr_t dest_addr;
  NetState_t state;
  uint32_t tx_cnt;
  IVCIS_Command_t last_cmd;
  volatile bool new_cmd;
} NetCtrl_t;

static NetCtrl_t s_net = {0};

/* --- 私有: UDP 接收回调 --- */

static void
net_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
            const ip_addr_t *addr,
            uint16_t port /* Raw 模式仅分配数据 pbuf, 不使用包头链 */) {
  (void)arg;
  (void)pcb;
  (void)addr;
  (void)port;
  if (!p)
    return;
  /* 至少 8 字节包头 (Magic + Type + Reserved) */
  if (p->tot_len >= 8U) {
    IVCIS_Command_t *cmd = (IVCIS_Command_t *)p->payload;
    if (cmd->magic == IVCR_MAGIC) {
      uint32_t cpy_len = (p->tot_len > sizeof(IVCIS_Command_t))
                             ? sizeof(IVCIS_Command_t)
                             : p->tot_len;
      memset(&s_net.last_cmd, 0, sizeof(IVCIS_Command_t));
      memcpy(&s_net.last_cmd, cmd, cpy_len);
      s_net.new_cmd = true;
      DBG_NET("Cmd RECV: type=0x%02X len=%u", cmd->cmd_type,
              (uint32_t)p->tot_len);
    }
  }
  pbuf_free(p);
}

/* --- 公共 API: 生命周期 --- */

int8_t Net_Client_Init(void) {
  if (s_net.upcb)
    udp_remove(s_net.upcb);
  s_net.upcb = udp_new();
  if (!s_net.upcb) {
    s_net.state = NET_ERROR;
    return -1;
  }

  IP4_ADDR(&s_net.dest_addr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2,
           DEST_IP_ADDR3);
  if (udp_bind(s_net.upcb, IP_ADDR_ANY, UDP_LOCAL_PORT) != ERR_OK) {
    udp_remove(s_net.upcb);
    s_net.upcb = NULL;
    s_net.state = NET_ERROR;
    return -1;
  }
  udp_recv(s_net.upcb, net_recv_cb, NULL);
  s_net.state = NET_READY;
  return 0;
}

/* --- 公共 API: 传输 --- */

void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id) {
  if ((s_net.state != NET_READY) || !pData || !len)
    return;
  /* 仅当 pData 不在非缓存 D2/D3 SRAM 区域时执行 D-Cache 清洗 */
  if ((uint32_t)pData < 0x30000000U) {
    SCB_CleanDCache_by_Addr((uint32_t *)((uint32_t)pData & ~31U),
                            (int32_t)(len + 31U));
  }

  /* Raw 模式: 满载有效载荷 */
  uint32_t cap = NET_MAX_UDP_PAYLOAD;
  uint16_t total = (uint16_t)((len + cap - 1U) / cap);
  uint32_t off = 0U, streak = 0U;
  s_net.state = NET_SENDING;

  for (uint16_t i = 0U; i < total; i++) {
    uint32_t clen = ((len - off) > cap) ? cap : (len - off);
    struct pbuf *d = pbuf_alloc(PBUF_RAW, (uint16_t)clen, PBUF_ROM);
    if (!d) {
      streak = 3;
      break;
    }

    d->payload = (void *)(pData + off);
    if (udp_sendto(s_net.upcb, d, &s_net.dest_addr, UDP_REMOTE_PORT) ==
        ERR_OK) {
      off += clen;
      streak = 0;
    } else {
      streak++;
    }
    pbuf_free(d);
    if (streak >= 3U)
      break;

    osDelay(1);
  }
  s_net.tx_cnt++;
  s_net.state = NET_READY;
  DBG_NET("Sent %lu bytes (%u chunks)", len, total);
}

/* --- Public API: Reception --- */

bool Net_Client_RecvCommand(IVCIS_Command_t *cmd) {
  if (!s_net.new_cmd || !cmd)
    return false;
  memcpy(cmd, &s_net.last_cmd, sizeof(IVCIS_Command_t));
  s_net.new_cmd = false;
  return true;
}

NetState_t Net_Client_GetState(void) { return s_net.state; }
uint32_t Net_Client_GetTxCount(void) { return s_net.tx_cnt; }
