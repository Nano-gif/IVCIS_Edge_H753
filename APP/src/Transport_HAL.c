/**
 * @file    Transport_HAL.c
 * @brief   通信通道抽象 — 注册表实现
 */

#include "Transport_HAL.h"
#include <stddef.h>

static const Transport_Ops_t *s_registry[TRANSPORT_MAX] = {NULL};

void Transport_Register(const Transport_Ops_t *ops) {
  if (ops && ops->type < TRANSPORT_MAX) {
    s_registry[ops->type] = ops;
  }
}

const Transport_Ops_t *Transport_Get(TransportType_t type) {
  if (type < TRANSPORT_MAX) return s_registry[type];
  return NULL;
}
