/**
 * @file    Transport_ETH.c
 * @brief   ETH (UDP Netconn) 后端 — Transport_HAL 实现
 */

#include "Transport_HAL.h"
#include "Net_Client.h"
#include "debug_config.h"

static int8_t eth_init(void) {
  return Net_Client_Init();
}

static TransResult_t eth_send(const uint8_t *data, uint32_t len, uint32_t id) {
  if (Net_Client_GetState() != NET_READY)
    return TRANS_ERR_INIT;
  Net_Client_SendImage((uint8_t *)data, len, id);
  return TRANS_OK;
}

static TransResult_t eth_recv(uint8_t *buf, uint32_t buf_size,
                              uint32_t *out_len, uint32_t timeout_ms) {
  (void)buf;
  (void)buf_size;
  (void)out_len;
  (void)timeout_ms;
  /* 接收由 NetRx 任务直接调用 Net_Client_RecvCommand，
     不经过 Transport 抽象（指令协议与通用数据流不同） */
  return TRANS_ERR_NO_IMPL;
}

static void eth_deinit(void) {
  /* Netconn 生命周期跟随系统，嵌入式不主动销毁 */
}

static const Transport_Ops_t s_eth_ops = {
    .type   = TRANSPORT_ETH,
    .init   = eth_init,
    .send   = eth_send,
    .recv   = eth_recv,
    .deinit = eth_deinit,
};

const Transport_Ops_t *Transport_ETH_GetOps(void) {
  return &s_eth_ops;
}
