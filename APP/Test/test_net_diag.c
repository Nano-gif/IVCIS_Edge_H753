/**
 * @file  test_net_diag.c
 * @brief Network diagnostics: ETH/PHY/LwIP status dump
 *
 * TEST_SELECT == 4
 * Runs after MX_LWIP_Init(), prints full network status.
 * Uses netif flags (updated by ethernet_link_thread) to
 * avoid MDIO bus contention and HardFault.
 */
#include "test_net_diag.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "lan8742.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "main.h"


/* External references */
extern ETH_HandleTypeDef heth;
extern lan8742_Object_t LAN8742;

/* ---- helpers ---- */
static const char *link_str(int32_t st) {
  switch (st) {
  case LAN8742_STATUS_100MBITS_FULLDUPLEX:
    return "100M-FD";
  case LAN8742_STATUS_100MBITS_HALFDUPLEX:
    return "100M-HD";
  case LAN8742_STATUS_10MBITS_FULLDUPLEX:
    return "10M-FD";
  case LAN8742_STATUS_10MBITS_HALFDUPLEX:
    return "10M-HD";
  case LAN8742_STATUS_LINK_DOWN:
    return "LINK_DOWN";
  default:
    return "READ_ERROR";
  }
}

static void print_netif(struct netif *nif) {
  if (!nif) {
    dbg_printf("  netif = NULL!\r\n");
    return;
  }
  dbg_printf("  IP=%s  flags=0x%02X [%s%s%s%s]\r\n", ipaddr_ntoa(&nif->ip_addr),
             nif->flags, (nif->flags & NETIF_FLAG_UP) ? "UP " : "",
             (nif->flags & NETIF_FLAG_LINK_UP) ? "LINK_UP " : "",
             (nif->flags & NETIF_FLAG_BROADCAST) ? "BCAST " : "",
             (nif->flags & NETIF_FLAG_ETHARP) ? "ETHARP" : "");
}

/* ---- main test ---- */
void Test_Net_Diag_Run(void) {
  dbg_printf("\r\n===== NET DIAG START =====\r\n");

  /* 1. ETH HAL status */
  dbg_printf("[1] ETH HAL state=%lu  err=%lu\r\n", (unsigned long)heth.gState,
             (unsigned long)heth.ErrorCode);

  /* 2. PHY initial state (one-shot, before link thread polls) */
  osDelay(200); /* let link thread settle */
  int32_t phy = LAN8742_GetLinkState(&LAN8742);
  dbg_printf("[2] PHY link: %s (%ld)\r\n", link_str(phy), (long)phy);

  /* 3. PHY registers via HAL */
  uint32_t bcr = 0, bsr = 0;
  HAL_ETH_ReadPHYRegister(&heth, 0, 0, &bcr);
  HAL_ETH_ReadPHYRegister(&heth, 0, 1, &bsr);
  dbg_printf("[3] BCR=0x%04lX  BSR=0x%04lX  AutoNeg=%s  Link=%s\r\n",
             (unsigned long)bcr, (unsigned long)bsr,
             (bsr & (1u << 5)) ? "DONE" : "NO",
             (bsr & (1u << 2)) ? "UP" : "DOWN");

  /* 4. DMA descriptors */
  dbg_printf("[4] DMA Tx=%p  Rx=%p\r\n", (void *)heth.Init.TxDesc,
             (void *)heth.Init.RxDesc);

  /* 5. netif snapshot */
  dbg_printf("[5] netif:");
  print_netif(netif_default);

  /* 6. Poll for LINK_UP via netif flags (safe, no MDIO) */
  dbg_printf("[6] Waiting for LINK_UP (max 10s)...\r\n");
  for (int i = 1; i <= 10; i++) {
    osDelay(1000);
    struct netif *nif = netif_default;
    if (nif && (nif->flags & NETIF_FLAG_LINK_UP)) {
      dbg_printf("  >> LINK UP at %ds!\r\n", i);
      print_netif(nif);
      dbg_printf("===== PING TEST READY =====\r\n");
      return;
    }
    dbg_printf("  %ds: still DOWN\r\n", i);
  }

  /* 7. Link never came up - read PHY one more time */
  dbg_printf("[7] LINK STILL DOWN after 10s\r\n");
  HAL_ETH_ReadPHYRegister(&heth, 0, 1, &bsr);
  dbg_printf("  BSR=0x%04lX  Link=%s  AutoNeg=%s\r\n", (unsigned long)bsr,
             (bsr & (1u << 2)) ? "UP" : "DOWN",
             (bsr & (1u << 5)) ? "DONE" : "NO");
  dbg_printf("===== NET DIAG END (FAIL) =====\r\n");
}
