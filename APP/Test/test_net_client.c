/**
 * @file    test_net_client.c
 * @brief   Net_Client TDD 测试: 分片 UDP 发送验证
 */

#include "test_net_client.h"
#include "Net_Client.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include <string.h>

/* Mock 1.5 KB Data (Forces 2 chunks) */
#define TEST_BUF_SIZE 1500
static uint8_t s_test_buf[TEST_BUF_SIZE];

void Test_Net_Client_Run(void) {
  DBG_INFO("=== TDD: Net_Client Start ===");

  /* Fill with dummy data */
  for (uint32_t i = 0; i < TEST_BUF_SIZE; i++) {
    s_test_buf[i] = (uint8_t)(i & 0xFF);
  }

  /* 1. Init */
  if (Net_Client_Init() != 0) {
    DBG_ERROR("Net_Client_Init FAIL");
    return;
  }

  /* 2. Wait for netif to be UP (background logic) */
  osDelay(2000);

  /* 3. Send Fragmented Image */
  DBG_INFO("Sending 1500B (Expect 2 chunks)...");
  Net_Client_SendImage(s_test_buf, TEST_BUF_SIZE, 0x1234);

  /* 4. Check State & Diagnostic */
  if (Net_Client_GetState() != NET_READY) {
    DBG_ERROR("Net state not READY after send");
  }
  Net_Client_Diagnostic();

  DBG_INFO("=== TDD: Net_Client Waiting for CMD... ===");

  /* 5. Infinite polling loop for TDD commands */
  IVCIS_Command_t cmd;
  while (1) {
    if (Net_Client_RecvCommand(&cmd)) {
      if (cmd.cmd_type == CMD_TYPE_VIOLATION) {
        DBG_INFO("RECV: Violation! Plate=%s", cmd.payload.violation.plate);
      } else if (cmd.cmd_type == CMD_TYPE_SERVO) {
        DBG_INFO("RECV: Servo Angle=%u", cmd.payload.servo.angle_deg);
      }
    }
    osDelay(100);
  }
}
