/**
 * @file    Net_Client_Util.c
 * @brief   Net_Client 辅助诊断工具
 */

#include "Net_Client.h"
#include "app_config.h"
#include "debug_config.h"
#include "stm32h7xx_hal.h"

extern ETH_HandleTypeDef heth;

void Net_Client_Diagnostic(void) {
  DBG_NET("=== HW Diag ===");
  DBG_NET("ETH State: 0x%lX", (uint32_t)heth.gState);
  DBG_NET("TX Desc:   0x%lX", heth.TxDescList.TxDesc[0]);
  DBG_NET("RX Desc:   0x%lX", heth.RxDescList.RxDesc[0]);
  DBG_NET("Frames Sent: %lu", Net_Client_GetTxCount());

  if (heth.TxDescList.TxDesc[0] < 0x30000000U) {
    DBG_ERROR("[Net] TX Desc NOT in D2 SRAM!");
  }
}
