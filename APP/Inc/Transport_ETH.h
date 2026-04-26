#ifndef TRANSPORT_ETH_H
#define TRANSPORT_ETH_H

#include "Transport_HAL.h"

/** 获取 ETH (UDP) 后端操作表，用于 Transport_Register */
const Transport_Ops_t *Transport_ETH_GetOps(void);

#endif /* TRANSPORT_ETH_H */
