/**
 * @file    test_net_client.c
 * @brief   TDD Test suite for Net_Client module
 */
#include "Net_Client.h"
#include "debug_config.h"
#include <stdio.h>

/* Mock or Stub if needed, but here we test actual Init */

void Test_Net_Client_Init(void) {
  DBG_INFO("[Test] Running Test_Net_Client_Init...");

  int32_t ret = Net_Client_Init();

  if (ret == 0) {
    DBG_INFO("[Test] Net_Client_Init PASS (Ret=0)");
  } else {
    DBG_ERROR("[Test] Net_Client_Init FAIL (Ret=%ld)", ret);
  }
}

void Test_Net_Client_State(void) {
  /* Accessing internal logic via public header if exposed,
     or observing side effects. Here checking if Diagnostic runs without crash
   */
  DBG_INFO("[Test] Running Test_Net_Client_Diagnostic...");
  Net_Client_Diagnostic();
  DBG_INFO("[Test] Diagnostic completed.");
}

void Test_Net_Client_Run(void) {
  DBG_INFO("=== Net_Client TDD Start ===");
  Test_Net_Client_Init();
  Test_Net_Client_State();
  DBG_INFO("=== Net_Client TDD End ===");
}
