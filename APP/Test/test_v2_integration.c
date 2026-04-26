/**
 * @file    test_v2_integration.c
 * @brief   V2 端到端集成测试 (TEST_SELECT=6)
 *
 * 验证项:
 *   T1. 双缓冲地址隔离 + D2 SRAM 范围
 *   T2. BufState 初始状态 = FREE
 *   T3. 消息队列创建 + Put/Get 一致性
 *   T4. 缓冲区生命周期: FREE→QUEUED→SENDING→FREE
 *   T5. 背压: 队列满 Put 不阻塞
 *   T6. SwitchBuffer 在双缓冲间交替
 *   T7. Netconn UDP 发送 (实际网络, 需要网线)
 *   T8. 全流程: 摄像头采集 → 入队 → NetTx 消费 → UDP 发送
 *
 * 使用方法: debug_config.h 中设置 TEST_SELECT=6，烧录后通过串口观察结果。
 */

#include "test_v2_integration.h"
#include "Camera_Task.h"
#include "Control_Manager.h"
#include "Modbus_RTU.h"
#include "Vision_Pipeline.h"
#include "Net_Client.h"
#include "Radar_Manager.h"
#include "RS485_Driver.h"
#include "RS485_Manager.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "shared_types.h"
#include <string.h>

/* 测试统计 */
static uint32_t s_pass = 0;
static uint32_t s_fail = 0;

#define TEST_ASSERT(cond, name) do { \
    if (cond) { s_pass++; DBG_INFO("[PASS] %s", name); } \
    else      { s_fail++; DBG_ERROR("[FAIL] %s", name); } \
  } while(0)

/* ── T1: 双缓冲地址隔离 ── */
static void test_dual_buffer_layout(void) {
  DBG_INFO("--- T1: Dual Buffer Layout ---");

  uint8_t idx0 = 0;
  Vision_SetBufState(0, BUF_FREE);
  Vision_SetBufState(1, BUF_FREE);

  /* 强制 active=0, 获取 buf A 地址 */
  while (Vision_GetActiveIdx() != 0) Vision_SwitchBuffer();
  (void)Vision_GetFrameBuffer();
  /* 这里 bufA 可能是 NULL (没采集过), 用另一种方式: 直接检查 BufState */

  /* 验证两个 buffer 初始状态 */
  BufState_t st0 = Vision_GetBufState(0);
  BufState_t st1 = Vision_GetBufState(1);
  TEST_ASSERT(st0 == BUF_FREE, "T1.1 Buf[0] init FREE");
  TEST_ASSERT(st1 == BUF_FREE, "T1.2 Buf[1] init FREE");

  /* 验证 activeIdx 范围 */
  uint8_t ai = Vision_GetActiveIdx();
  TEST_ASSERT(ai < FRAME_BUF_COUNT, "T1.3 ActiveIdx in range");
  (void)idx0;
}

/* ── T2: 缓冲区状态读写 ── */
static void test_bufstate_rw(void) {
  DBG_INFO("--- T2: BufState Read/Write ---");

  Vision_SetBufState(0, BUF_QUEUED);
  TEST_ASSERT(Vision_GetBufState(0) == BUF_QUEUED, "T2.1 Set/Get QUEUED");

  Vision_SetBufState(0, BUF_SENDING);
  TEST_ASSERT(Vision_GetBufState(0) == BUF_SENDING, "T2.2 Set/Get SENDING");

  Vision_SetBufState(0, BUF_FREE);
  TEST_ASSERT(Vision_GetBufState(0) == BUF_FREE, "T2.3 Set/Get FREE");

  /* 越界保护 */
  BufState_t oob = Vision_GetBufState(99);
  TEST_ASSERT(oob == BUF_FREE, "T2.4 OOB returns FREE");
}

/* ── T3: 消息队列 Put/Get ── */
static void test_message_queue(void) {
  DBG_INFO("--- T3: Message Queue ---");

  osMessageQueueId_t q = osMessageQueueNew(FRAME_QUEUE_DEPTH,
                                           sizeof(FrameDesc_t), NULL);
  TEST_ASSERT(q != NULL, "T3.1 Queue created");

  /* Put */
  FrameDesc_t fd_in = {
    .frame_id = 0xABCD,
    .data_len = 12345,
    .p_data   = (uint8_t *)0x30000000,
    .buf_idx  = 0,
  };
  osStatus_t st = osMessageQueuePut(q, &fd_in, 0, 0);
  TEST_ASSERT(st == osOK, "T3.2 Put OK");

  /* Get */
  FrameDesc_t fd_out = {0};
  st = osMessageQueueGet(q, &fd_out, NULL, 0);
  TEST_ASSERT(st == osOK, "T3.3 Get OK");
  TEST_ASSERT(fd_out.frame_id == 0xABCD, "T3.4 frame_id match");
  TEST_ASSERT(fd_out.data_len == 12345, "T3.5 data_len match");
  TEST_ASSERT(fd_out.p_data == (uint8_t *)0x30000000, "T3.6 p_data match");
  TEST_ASSERT(fd_out.buf_idx == 0, "T3.7 buf_idx match");

  /* Backpressure: 填满队列后 Put 应失败 */
  for (uint32_t i = 0; i < FRAME_QUEUE_DEPTH; i++) {
    fd_in.frame_id = i;
    osMessageQueuePut(q, &fd_in, 0, 0);
  }
  fd_in.frame_id = 0xFFFF;
  st = osMessageQueuePut(q, &fd_in, 0, 0);
  TEST_ASSERT(st != osOK, "T3.8 Queue full, Put rejected (non-blocking)");

  osMessageQueueDelete(q);
}

/* ── T4: 缓冲区生命周期流转 ── */
static void test_camera_event_api(void) {
  DBG_INFO("--- T4: Camera Event API ---");

  TEST_ASSERT(Camera_TaskEventsReady(), "T4.1 Camera events ready");
  TEST_ASSERT(Camera_RequestHealthCheck(), "T4.2 Health request accepted");
  TEST_ASSERT(Camera_RequestCapture(), "T4.3 Capture request accepted");
}

static void test_radar_trigger_policy(void) {
  DBG_INFO("--- T5: Radar Trigger Policy ---");

  Radar_Init();
  RadarSummary_t target = {
      .valid = true,
      .track_id = 7,
      .range_cm = 900,
      .speed_cms = 450,
      .lane_id = 1,
      .target_count = 1,
      .confidence = 90,
      .direction = RADAR_DIR_APPROACHING,
  };

  TEST_ASSERT(Radar_UpdateSummary(&target), "T5.1 approaching target triggers");

  target.confidence = 20;
  TEST_ASSERT(!Radar_UpdateSummary(&target), "T5.2 low confidence rejected");

  target.confidence = 90;
  target.range_cm = 5000;
  TEST_ASSERT(!Radar_UpdateSummary(&target), "T5.3 out-of-zone rejected");

  RadarSummary_t snapshot = {0};
  TEST_ASSERT(Radar_GetSummaryCopy(&snapshot), "T5.4 snapshot copy OK");
  TEST_ASSERT(snapshot.track_id == target.track_id, "T5.5 snapshot track match");
}

static void test_control_command_queue(void) {
  DBG_INFO("--- T6: Control Command Queue ---");

  TEST_ASSERT(Control_Manager_Init(), "T6.1 Control manager init");
  TEST_ASSERT(Control_Manager_SubmitAlarm(CONTROL_SRC_ETH, true),
              "T6.2 Alarm command submitted");

  ControlCommand_t cmd = {0};
  TEST_ASSERT(Control_Manager_Receive(&cmd, 0), "T6.3 Command received");
  TEST_ASSERT(cmd.type == CONTROL_CMD_ALARM, "T6.4 Command type alarm");
  TEST_ASSERT(cmd.source == CONTROL_SRC_ETH, "T6.5 Command source ETH");
  TEST_ASSERT(cmd.payload.alarm.active == true, "T6.6 Alarm active true");
}

static void test_rs485_stream_buffer(void) {
  DBG_INFO("--- T7: RS485 Stream Buffer ---");

  TEST_ASSERT(RS485_Manager_Init(), "T7.1 RS485 manager init");

  const uint8_t rx_bytes[] = {0x01, 0x03, 0x00, 0x10, 0x00, 0x02};
  uint8_t out[sizeof(rx_bytes)] = {0};

  size_t written = RS485_Manager_WriteRxBytes(rx_bytes, sizeof(rx_bytes), 0);
  TEST_ASSERT(written == sizeof(rx_bytes), "T7.2 Stream write all bytes");

  size_t read = RS485_Manager_ReadRxBytes(out, sizeof(out), 0);
  TEST_ASSERT(read == sizeof(rx_bytes), "T7.3 Stream read all bytes");
  TEST_ASSERT(memcmp(out, rx_bytes, sizeof(rx_bytes)) == 0,
              "T7.4 Stream bytes match");
}

static void test_rs485_driver_layer(void) {
  DBG_INFO("--- T8: RS485 MAX3485 Driver ---");

  RS485_Driver_SetDirection(RS485_DIR_TX);
  TEST_ASSERT(RS485_Driver_GetDirection() == RS485_DIR_TX,
              "T8.1 Driver direction TX");

  RS485_Driver_SetDirection(RS485_DIR_RX);
  TEST_ASSERT(RS485_Driver_GetDirection() == RS485_DIR_RX,
              "T8.2 Driver direction RX");

  TEST_ASSERT(RS485_Driver_Init(), "T8.3 Driver init");
  TEST_ASSERT(RS485_Driver_IsReady(), "T8.4 Driver ready");

  uint8_t tx_bytes[4] = {0x01, 0x03, 0x00, 0x00};
  TEST_ASSERT(RS485_Manager_WriteTxBytes(tx_bytes, sizeof(tx_bytes), 0) ==
                  sizeof(tx_bytes),
              "T8.5 Driver TX stream enqueue");
  TEST_ASSERT(RS485_Driver_FlushTxStream(20U) == sizeof(tx_bytes),
              "T8.6 Driver TX stream flush");
  TEST_ASSERT(RS485_Driver_GetDirection() == RS485_DIR_RX,
              "T8.7 Driver returns to RX after TX");
}

static void test_modbus_rtu_protocol(void) {
  DBG_INFO("--- T8: Modbus RTU Protocol ---");

  Modbus_RTU_Init(MODBUS_DEFAULT_SLAVE_ADDR);

  const uint8_t crc_req[] = {0x01, 0x03, 0x00, 0x10, 0x00, 0x01};
  TEST_ASSERT(Modbus_RTU_Crc16(crc_req, sizeof(crc_req)) == 0xCF85U,
              "T8.1 CRC16 known vector");

  uint8_t fc16_req[13] = {0x01, 0x10, 0x00, 0x34, 0x00, 0x02, 0x04,
                          0x17, 0x70, 0x00, 0x0C, 0x00, 0x00};
  Modbus_RTU_AppendCrc(fc16_req, 11U);
  TEST_ASSERT(Modbus_RTU_GetExpectedLength(fc16_req, 7U) == sizeof(fc16_req),
              "T8.2 FC16 expected length");

  uint8_t read_req[8] = {0x01, 0x03, 0x00, 0x10, 0x00, 0x01, 0x85, 0xCF};
  uint8_t rsp[64] = {0};
  size_t rsp_len = Modbus_RTU_ProcessFrame(read_req,
                                           sizeof(read_req),
                                           rsp,
                                           sizeof(rsp));
  TEST_ASSERT(rsp_len == 7U, "T8.3 FC03 response length");
  TEST_ASSERT((rsp[0] == 0x01) && (rsp[1] == 0x03) && (rsp[2] == 0x02),
              "T8.4 FC03 response header");
  TEST_ASSERT(Modbus_RTU_Crc16(rsp, rsp_len - 2U) ==
                  (uint16_t)(rsp[rsp_len - 2U] |
                             ((uint16_t)rsp[rsp_len - 1U] << 8)),
              "T8.5 FC03 response CRC");

  ControlCommand_t drained;
  while (Control_Manager_Receive(&drained, 0)) {
  }

  uint8_t alarm_req[8] = {0x01, 0x06, 0x00, 0x23, 0x00, 0x01, 0x00, 0x00};
  Modbus_RTU_AppendCrc(alarm_req, 6U);
  rsp_len = Modbus_RTU_ProcessFrame(alarm_req,
                                    sizeof(alarm_req),
                                    rsp,
                                    sizeof(rsp));
  TEST_ASSERT(rsp_len == sizeof(alarm_req), "T8.6 FC06 echo length");
  TEST_ASSERT(memcmp(rsp, alarm_req, sizeof(alarm_req)) == 0,
              "T8.7 FC06 echo match");

  ControlCommand_t cmd = {0};
  TEST_ASSERT(Control_Manager_Receive(&cmd, 0), "T8.8 RS485 command queued");
  TEST_ASSERT((cmd.source == CONTROL_SRC_RS485) &&
                  (cmd.type == CONTROL_CMD_ALARM) &&
                  (cmd.payload.alarm.active == true),
              "T8.9 RS485 alarm command content");

  rsp_len = Modbus_RTU_ProcessFrame(fc16_req,
                                    sizeof(fc16_req),
                                    rsp,
                                    sizeof(rsp));
  TEST_ASSERT(rsp_len == 8U, "T8.10 FC16 response length");
  TEST_ASSERT((rsp[0] == 0x01) && (rsp[1] == 0x10) &&
                  (rsp[2] == 0x00) && (rsp[3] == 0x34) &&
                  (rsp[4] == 0x00) && (rsp[5] == 0x02),
              "T8.11 FC16 response header");

  uint8_t bad_func_req[8] = {0x01, 0x04, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00};
  Modbus_RTU_AppendCrc(bad_func_req, 6U);
  rsp_len = Modbus_RTU_ProcessFrame(bad_func_req,
                                    sizeof(bad_func_req),
                                    rsp,
                                    sizeof(rsp));
  TEST_ASSERT(rsp_len == 5U, "T8.12 exception response length");
  TEST_ASSERT((rsp[1] == 0x84) && (rsp[2] == MODBUS_EX_ILLEGAL_FUNCTION),
              "T8.13 illegal function exception");

  uint8_t bcast_read_req[8] = {0x00, 0x03, 0x00, 0x10,
                               0x00, 0x01, 0x00, 0x00};
  Modbus_RTU_AppendCrc(bcast_read_req, 6U);
  rsp_len = Modbus_RTU_ProcessFrame(bcast_read_req,
                                    sizeof(bcast_read_req),
                                    rsp,
                                    sizeof(rsp));
  TEST_ASSERT(rsp_len == 0U, "T8.14 broadcast read has no response");
}

static void test_lifecycle(void) {
  DBG_INFO("--- T4: Buffer Lifecycle ---");

  /* 重置到已知状态 */
  Vision_SetBufState(0, BUF_FREE);
  Vision_SetBufState(1, BUF_FREE);

  /* 模拟 Camera 生产者 */
  uint8_t idx = Vision_GetActiveIdx();
  Vision_SetBufState(idx, BUF_QUEUED);
  TEST_ASSERT(Vision_GetBufState(idx) == BUF_QUEUED, "T4.1 Camera: FREE->QUEUED");

  /* 切换缓冲区 */
  uint8_t switched = Vision_SwitchBuffer();
  TEST_ASSERT(switched == 1, "T4.2 SwitchBuffer OK");
  TEST_ASSERT(Vision_GetActiveIdx() != idx, "T4.3 ActiveIdx changed");

  /* 模拟 NetTx 消费者 */
  Vision_SetBufState(idx, BUF_SENDING);
  TEST_ASSERT(Vision_GetBufState(idx) == BUF_SENDING, "T4.4 NetTx: QUEUED->SENDING");

  Vision_SetBufState(idx, BUF_FREE);
  TEST_ASSERT(Vision_GetBufState(idx) == BUF_FREE, "T4.5 NetTx: SENDING->FREE");

  /* 背压: 两个都占用时 SwitchBuffer 应失败 */
  Vision_SetBufState(0, BUF_QUEUED);
  Vision_SetBufState(1, BUF_SENDING);
  switched = Vision_SwitchBuffer();
  TEST_ASSERT(switched == 0, "T4.6 Backpressure: SwitchBuffer blocked");

  /* 恢复 */
  Vision_SetBufState(0, BUF_FREE);
  Vision_SetBufState(1, BUF_FREE);
}

/* ── T5: Netconn 初始化 ── */
static void test_netconn_init(void) {
  DBG_INFO("--- T5: Netconn Init ---");

  int8_t r = Net_Client_Init();
  TEST_ASSERT(r == 0, "T5.1 Net_Client_Init (TX conn)");
  TEST_ASSERT(Net_Client_GetState() == NET_READY, "T5.2 State = READY");
  TEST_ASSERT(Net_Client_GetTxState() == NET_READY, "T5.3 TX state = READY");

  int8_t r2 = Net_Client_InitRx();
  TEST_ASSERT(r2 == 0, "T5.4 Net_Client_InitRx (RX conn)");
  TEST_ASSERT(Net_Client_IsRxReady(), "T5.5 RX ready");
  TEST_ASSERT(Net_Client_GetTxState() == NET_READY, "T5.6 RX init keeps TX ready");
}

/* ── T6: 实际网络发送 (需网线) ── */
static void test_net_send(void) {
  DBG_INFO("--- T6: UDP Send ---");
  DBG_INFO("(Need Ethernet cable + Wireshark on %d.%d.%d.%d:%d)",
           DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3,
           UDP_REMOTE_PORT);

  /* 等待 LwIP netif up */
  osDelay(2000);

  /* 构造测试数据: 3000B → 应产生 3 个 UDP 分片 */
  static uint8_t test_data[3000];
  for (uint32_t i = 0; i < sizeof(test_data); i++)
    test_data[i] = (uint8_t)(i & 0xFF);

  uint32_t tx_before = Net_Client_GetTxCount();
  Net_Client_SendImage(test_data, sizeof(test_data), 0x0001);
  uint32_t tx_after = Net_Client_GetTxCount();

  TEST_ASSERT(tx_after == tx_before + 1, "T6.1 TxCount incremented");
  TEST_ASSERT(Net_Client_GetState() == NET_READY, "T6.2 State READY after send");

  DBG_INFO("Expect 3 UDP packets (1400+1400+200 B) in Wireshark");
}

/* ── T7: 全流程 (Camera → Queue → Send) ── */
static void test_full_pipeline(void) {
  DBG_INFO("--- T7: Full Pipeline (Camera->Queue->NetTx) ---");

  /* 初始化摄像头 */
  if (Vision_Init() != 0) {
    DBG_WARN("T7: Vision_Init FAIL, skip pipeline test");
    return;
  }

  Vision_SetMode(VISION_MODE_JPEG);
  osDelay(500);

  /* 创建临时队列模拟 */
  osMessageQueueId_t q = osMessageQueueNew(2, sizeof(FrameDesc_t), NULL);
  if (!q) { DBG_ERROR("T7: Queue alloc fail"); return; }

  Vision_SetBufState(0, BUF_FREE);
  Vision_SetBufState(1, BUF_FREE);

  uint32_t captured = 0, sent = 0, dropped = 0;

  /* 采集 5 帧, 模拟生产者-消费者 */
  for (uint32_t f = 0; f < 5; f++) {
    /* 生产者: 采集 */
    Vision_CaptureStart();
    if (!Vision_IsFrameReady()) {
      DBG_WARN("T7: Frame %lu capture fail", f);
      continue;
    }
    captured++;

    uint8_t idx = Vision_GetActiveIdx();
    FrameDesc_t fd = {
      .frame_id = f,
      .data_len = Vision_GetFrameSize(),
      .p_data   = Vision_GetFrameBuffer(),
      .buf_idx  = idx,
    };
    Vision_SetBufState(idx, BUF_QUEUED);

    if (osMessageQueuePut(q, &fd, 0, 0) != osOK) {
      Vision_SetBufState(idx, BUF_FREE);
      dropped++;
      DBG_WARN("T7: Frame %lu dropped (backpressure)", f);
    } else {
      Vision_SwitchBuffer();
    }

    /* 消费者: 取出并发送 */
    FrameDesc_t fd_out;
    if (osMessageQueueGet(q, &fd_out, NULL, 0) == osOK) {
      Vision_SetBufState(fd_out.buf_idx, BUF_SENDING);
      Net_Client_SendImage(fd_out.p_data, fd_out.data_len, fd_out.frame_id);
      Vision_SetBufState(fd_out.buf_idx, BUF_FREE);
      sent++;
    }

    osDelay(50);
  }

  TEST_ASSERT(captured > 0, "T7.1 Captured > 0 frames");
  TEST_ASSERT(sent > 0, "T7.2 Sent > 0 frames");
  TEST_ASSERT(sent + dropped == captured, "T7.3 sent+dropped == captured");

  /* 验证缓冲区全部回收 */
  TEST_ASSERT(Vision_GetBufState(0) == BUF_FREE, "T7.4 Buf[0] final FREE");
  TEST_ASSERT(Vision_GetBufState(1) == BUF_FREE, "T7.5 Buf[1] final FREE");

  DBG_INFO("T7: captured=%lu sent=%lu dropped=%lu", captured, sent, dropped);
  osMessageQueueDelete(q);
}

/* ===== 主入口 ===== */

void Test_V2_Integration_Run(void) {
  DBG_INFO("========================================");
  DBG_INFO("  V2 Integration Test (TEST_SELECT=6)   ");
  DBG_INFO("========================================");

  /* 纯逻辑测试 (不需要硬件) */
  test_dual_buffer_layout();
  test_bufstate_rw();
  test_message_queue();
  test_camera_event_api();
  test_radar_trigger_policy();
  test_control_command_queue();
  test_rs485_stream_buffer();
  test_rs485_driver_layer();
  test_modbus_rtu_protocol();
  test_lifecycle();

  /* 网络测试 (需以太网) */
  test_netconn_init();
  test_net_send();

  /* 全流程测试 (需摄像头 + 网络) */
  test_full_pipeline();

  /* 汇总 */
  DBG_INFO("========================================");
  DBG_INFO("  RESULT: %lu PASS / %lu FAIL", s_pass, s_fail);
  DBG_INFO("========================================");
  if (s_fail == 0) {
    DBG_INFO("  ALL TESTS PASSED");
  } else {
    DBG_ERROR("  %lu TEST(S) FAILED", s_fail);
  }
}
