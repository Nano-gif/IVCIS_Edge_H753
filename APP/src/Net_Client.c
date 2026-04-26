/**
 * @file    Net_Client.c
 * @brief   UDP Netconn TX/RX client with separated task-owned state.
 */

#include "Net_Client.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "lwip/api.h"
#include "lwip/netbuf.h"
#include "stm32h7xx_hal.h"
#include <string.h>

typedef struct {
  struct netconn *conn;
  ip_addr_t dest_addr;
  NetState_t state;
  uint32_t tx_cnt;
} NetTxCtrl_t;

typedef struct {
  struct netconn *conn;
  bool ready;
} NetRxCtrl_t;

static NetTxCtrl_t s_net_tx = {0};
static NetRxCtrl_t s_net_rx = {0};
static osMutexId_t s_net_lock;

static bool net_lock(void) {
  if (s_net_lock == NULL) {
    s_net_lock = osMutexNew(NULL);
  }
  if (s_net_lock == NULL) {
    return true;
  }
  return (osMutexAcquire(s_net_lock, osWaitForever) == osOK);
}

static void net_unlock(void) {
  if (s_net_lock != NULL) {
    (void)osMutexRelease(s_net_lock);
  }
}

static void net_set_tx_state(NetState_t state) {
  bool locked = net_lock();
  s_net_tx.state = state;
  if (locked) {
    net_unlock();
  }
}

static void net_set_rx_ready(bool ready) {
  bool locked = net_lock();
  s_net_rx.ready = ready;
  if (locked) {
    net_unlock();
  }
}

int8_t Net_Client_Init(void) {
  if (s_net_tx.conn != NULL) {
    net_set_tx_state(NET_READY);
    return 0;
  }

  s_net_tx.conn = netconn_new(NETCONN_UDP);
  if (s_net_tx.conn == NULL) {
    net_set_tx_state(NET_ERROR);
    return -1;
  }

  IP4_ADDR(&s_net_tx.dest_addr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2,
           DEST_IP_ADDR3);
  net_set_tx_state(NET_READY);
  DBG_NET("TX conn ready");
  return 0;
}

int8_t Net_Client_InitRx(void) {
  if (s_net_rx.conn != NULL) {
    net_set_rx_ready(true);
    return 0;
  }

  s_net_rx.conn = netconn_new(NETCONN_UDP);
  if (s_net_rx.conn == NULL) {
    DBG_ERROR("[Net] RX conn alloc fail");
    return -1;
  }

  if (netconn_bind(s_net_rx.conn, IP_ADDR_ANY, UDP_LOCAL_PORT) != ERR_OK) {
    netconn_delete(s_net_rx.conn);
    s_net_rx.conn = NULL;
    net_set_rx_ready(false);
    DBG_ERROR("[Net] RX bind fail");
    return -1;
  }

  net_set_rx_ready(true);
  DBG_NET("RX conn ready, port %u", UDP_LOCAL_PORT);
  return 0;
}

void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id) {
  (void)frame_id;
  if ((Net_Client_GetTxState() != NET_READY) || (s_net_tx.conn == NULL) ||
      (pData == NULL) || (len == 0U)) {
    return;
  }

  if ((uint32_t)pData < 0x30000000U) {
    SCB_CleanDCache_by_Addr((uint32_t *)((uint32_t)pData & ~31U),
                            (int32_t)(len + 31U));
  }

  uint32_t cap = NET_MAX_UDP_PAYLOAD;
  uint16_t total = (uint16_t)((len + cap - 1U) / cap);
  uint32_t off = 0U;
  uint32_t streak = 0U;
  net_set_tx_state(NET_SENDING);

  for (uint16_t i = 0U; i < total; i++) {
    uint32_t clen = ((len - off) > cap) ? cap : (len - off);
    struct netbuf *nb = netbuf_new();
    if (nb == NULL) {
      streak = 3U;
      break;
    }

    if (netbuf_ref(nb, pData + off, (uint16_t)clen) != ERR_OK) {
      netbuf_delete(nb);
      streak++;
      if (streak >= 3U) {
        break;
      }
      continue;
    }

    if (netconn_sendto(s_net_tx.conn, nb, &s_net_tx.dest_addr,
                       UDP_REMOTE_PORT) == ERR_OK) {
      off += clen;
      streak = 0U;
    } else {
      streak++;
    }

    netbuf_delete(nb);
    if (streak >= 3U) {
      break;
    }

    osDelay(1);
  }

  bool locked = net_lock();
  s_net_tx.tx_cnt++;
  s_net_tx.state = NET_READY;
  if (locked) {
    net_unlock();
  }
  DBG_NET("Sent %lu bytes (%u chunks)", len, total);
}

bool Net_Client_RecvCommand(IVCIS_Command_t *cmd) {
  if ((s_net_rx.conn == NULL) || (cmd == NULL)) {
    return false;
  }

  struct netbuf *nb = NULL;
  err_t err = netconn_recv(s_net_rx.conn, &nb);
  if ((err != ERR_OK) || (nb == NULL)) {
    return false;
  }

  void *data = NULL;
  uint16_t data_len = 0U;
  netbuf_data(nb, &data, &data_len);

  bool got_cmd = false;
  if (data_len >= 8U) {
    IVCIS_Command_t *raw = (IVCIS_Command_t *)data;
    if (raw->magic == IVCR_MAGIC) {
      uint16_t cpy = (data_len > sizeof(IVCIS_Command_t))
                         ? (uint16_t)sizeof(IVCIS_Command_t)
                         : data_len;
      memset(cmd, 0, sizeof(IVCIS_Command_t));
      memcpy(cmd, raw, cpy);
      got_cmd = true;
      DBG_NET("Cmd RECV: type=0x%02X len=%u", raw->cmd_type, data_len);
    }
  }

  netbuf_delete(nb);
  return got_cmd;
}

NetState_t Net_Client_GetTxState(void) {
  bool locked = net_lock();
  NetState_t state = s_net_tx.state;
  if (locked) {
    net_unlock();
  }
  return state;
}

NetState_t Net_Client_GetState(void) {
  return Net_Client_GetTxState();
}

bool Net_Client_IsRxReady(void) {
  bool locked = net_lock();
  bool ready = s_net_rx.ready;
  if (locked) {
    net_unlock();
  }
  return ready;
}

uint32_t Net_Client_GetTxCount(void) {
  bool locked = net_lock();
  uint32_t tx_cnt = s_net_tx.tx_cnt;
  if (locked) {
    net_unlock();
  }
  return tx_cnt;
}
