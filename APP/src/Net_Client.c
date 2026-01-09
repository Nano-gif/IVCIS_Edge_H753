#include "Net_Client.h"
#include "app_config.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "lwip/etharp.h"
#include "lwip/netif.h"
#include <string.h>
#include <stdio.h>

/* 引用外部以太网句柄 */
extern ETH_HandleTypeDef heth;

NetCtrl_t g_net_ctrl = {0};

int8_t Net_Client_Init(void) {
    g_net_ctrl.upcb = udp_new();
    if (g_net_ctrl.upcb == NULL) {
        g_net_ctrl.state = NET_ERROR;
        return -1;
    }
    IP4_ADDR(&g_net_ctrl.dest_addr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3);
    udp_bind(g_net_ctrl.upcb, IP_ADDR_ANY, UDP_LOCAL_PORT);
    
    /* 强制发起 ARP 请求，确保知道目标 MAC */
    struct netif *netif = netif_default;
    if (netif != NULL) {
        etharp_request(netif, &g_net_ctrl.dest_addr);
        printf("[NET] ARP request sent to %d.%d.%d.%d\r\n",
               DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3);
    }
    
    g_net_ctrl.state = NET_READY;
    return 0;
}

/**
 * @brief  诊断函数：严格基于官方库结构体成员进行硬件自检
 */
void Net_Client_Diagnostic(void) {
    /* 1. 获取全局状态 */
    uint32_t current_state = (uint32_t)heth.gState;

    /* 2. 读取运行时的描述符地址 (取列表中的第一个地址) */
    uint32_t run_tx_base = heth.TxDescList.TxDesc[0];
    uint32_t run_rx_base = heth.RxDescList.RxDesc[0];

    /* 3. 获取索引状态 */
    uint32_t tx_idx = heth.TxDescList.CurTxDesc;
    uint32_t rx_idx = heth.RxDescList.RxDescIdx;

    printf("\r\n[Net_Diag] === Hardware Status ===\r\n");
    printf("State: 0x%lX (0x40=STARTED)\r\n", current_state);
    printf("TX_Base: 0x%lX | RX_Base: 0x%lX\r\n", run_tx_base, run_rx_base);
    printf("TX_Idx: %ld | RX_Idx: %ld\r\n", tx_idx, rx_idx);
    printf("TX_Frames: %ld | Net_State: %d\r\n", g_net_ctrl.tx_frame_count, (int)g_net_ctrl.state);

    /* 4. [New] 硬件寄存器直接诊断 (解决地址漂移) */
    volatile uint32_t hw_tx_base = heth.Instance->DMACTDLAR;
    volatile uint32_t hw_rx_base = heth.Instance->DMACRDLAR;

    printf("HW_TX_Base: 0x%lX | HW_RX_Base: 0x%lX\r\n", hw_tx_base, hw_rx_base);

    /* 5. 关键警告与自动修正 */
    if (run_tx_base < 0x30000000) {
        printf("!!! FATAL: Handle Base is pointing to D1 SRAM (0x24...). Check Linker/Ethernetif!\r\n");
    }

    /* 强制重定向逻辑 (Action Plan requirements) */
    if (hw_tx_base < 0x30000000 && run_tx_base >= 0x30000000) {
        printf("!!! WARN: Hardware Drift Detected! Forcing Register Redirect...\r\n");
        heth.Instance->DMACTDLAR = run_tx_base;
        heth.Instance->DMACRDLAR = run_rx_base;
        printf("Redirected -> HW_TX: 0x%lX\r\n", heth.Instance->DMACTDLAR);
    }
}

void Net_Client_SendImage(uint8_t *pData, uint32_t len, uint32_t frame_id) {
    printf("[NET] SendImage called: state=%d, len=%ld\r\n", (int)g_net_ctrl.state, len);
    if (g_net_ctrl.state != NET_READY || pData == NULL || len == 0) {
        printf("[NET] SKIP: state not ready or invalid params\r\n");
        return;
    }

    struct pbuf *ptr_pbuf;
    uint32_t bytes_left = len;
    uint32_t current_offset = 0;
    uint32_t chunk_size;
    const uint32_t max_udp_payload = 1400;
    uint32_t sent_ok = 0, sent_fail = 0;
    err_t err;

    g_net_ctrl.state = NET_SENDING;
    SCB_CleanDCache_by_Addr((uint32_t*)pData, len);

    while (bytes_left > 0) {
        chunk_size = (bytes_left > max_udp_payload) ? max_udp_payload : bytes_left;
        ptr_pbuf = pbuf_alloc(PBUF_TRANSPORT, chunk_size, PBUF_ROM);
        if (ptr_pbuf != NULL) {
            ptr_pbuf->payload = (void *)(pData + current_offset);
            err = udp_sendto(g_net_ctrl.upcb, ptr_pbuf, &g_net_ctrl.dest_addr, UDP_REMOTE_PORT);
            if (err == ERR_OK) {
                bytes_left -= chunk_size;
                current_offset += chunk_size;
                sent_ok++;
            } else {
                sent_fail++;
            }
            pbuf_free(ptr_pbuf);
        } else {
            sent_fail++;
            break;
        }
    }
    printf("[NET] Frame %ld: %ld OK, %ld FAIL\r\n", frame_id, sent_ok, sent_fail);
    g_net_ctrl.tx_frame_count++;
    g_net_ctrl.state = NET_READY;
}
