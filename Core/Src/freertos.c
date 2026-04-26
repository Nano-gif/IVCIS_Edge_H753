/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : freertos.c
 * @brief          : 任务调度与系统逻辑核心 (V2: 生产者-消费者并行架构)
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "task.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Alarm_Handler.h"
#include "Auto_Exposure.h"
#include "Camera_Task.h"
#include "Control_Manager.h"
#include "Modbus_RTU.h"
#include "Motion_Detect.h"
#include "Net_Client.h"
#include "Power_Manager.h"
#include "Radar_Manager.h"
#include "RS485_Driver.h"
#include "RS485_Manager.h"
#include "Servo_Control.h"
#include "Vision_Pipeline.h"
#include "app_config.h"
#include "debug_config.h"
#include "ov5640.h"
#include "shared_types.h"
#include <string.h>

#if (TEST_SELECT == 1)
#include "test_mode_switch.h"
#elif (TEST_SELECT == 2)
#include "test_motion_detect.h"
#elif (TEST_SELECT == 3)
#include "test_power_manager.h"
#elif (TEST_SELECT == 4)
#include "test_net_diag.h"
#elif (TEST_SELECT == 5)
#include "test_net_client.h"
#elif (TEST_SELECT == 6)
#include "test_v2_integration.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAMERA_EVT_CAPTURE (1UL << 0)
#define CAMERA_EVT_HEALTH (1UL << 1)
#define CAMERA_EVT_ANY (CAMERA_EVT_CAPTURE | CAMERA_EVT_HEALTH)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static osMessageQueueId_t s_frame_q; /* Camera -> NetTx 帧队列 */
static osEventFlagsId_t s_camera_evt;
/* USER CODE END Variables */
/* Definitions for Task_Camera */
osThreadId_t Task_CameraHandle;
const osThreadAttr_t Task_Camera_attributes = {
    .name = "Task_Camera",
    .stack_size = 2048 * 4,
    .priority = (osPriority_t)osPriorityHigh,
};
/* Definitions for Task_Net */
osThreadId_t Task_NetHandle;
const osThreadAttr_t Task_Net_attributes = {
    .name = "Task_NetTx",
    .stack_size = 2048 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
#if (TEST_SELECT == 0)
static uint32_t Camera_WaitEvents(void);
#endif
static void StartNetRxTask(void *argument);
static void StartRadarTask(void *argument);
static void StartIdlePowerTask(void *argument);
#if (TEST_SELECT == 0)
static void StartControlTask(void *argument);
static void StartRS485Task(void *argument);
#endif

static const osThreadAttr_t Task_NetRx_attributes = {
    .name = "Task_NetRx",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t Task_Radar_attributes = {
    .name = "Task_Radar",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t Task_IdlePower_attributes = {
    .name = "Task_IdlePower",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityLow,
};

#if (TEST_SELECT == 0)
static const osThreadAttr_t Task_Control_attributes = {
    .name = "Task_Control",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

static const osThreadAttr_t Task_RS485_attributes = {
    .name = "Task_RS485",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
#endif
/* USER CODE END FunctionPrototypes */

void StartCameraTask(void *argument);
void StartNetTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  s_frame_q =
      osMessageQueueNew(FRAME_QUEUE_DEPTH, sizeof(FrameDesc_t), NULL);
  s_camera_evt = osEventFlagsNew(NULL);
  (void)Control_Manager_Init();
  (void)RS485_Manager_Init();
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_Camera */
  Task_CameraHandle =
      osThreadNew(StartCameraTask, NULL, &Task_Camera_attributes);

  /* creation of Task_NetTx */
  Task_NetHandle = osThreadNew(StartNetTask, NULL, &Task_Net_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  osThreadNew(StartNetRxTask, NULL, &Task_NetRx_attributes);
  osThreadNew(StartRadarTask, NULL, &Task_Radar_attributes);
  osThreadNew(StartIdlePowerTask, NULL, &Task_IdlePower_attributes);
#if (TEST_SELECT == 0)
  osThreadNew(StartControlTask, NULL, &Task_Control_attributes);
  osThreadNew(StartRS485Task, NULL, &Task_RS485_attributes);
#endif
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartCameraTask */
/**
 * @brief  Camera 任务 (生产者): 双缓冲采集 + 帧入队
 */
/* USER CODE END Header_StartCameraTask */
void StartCameraTask(void *argument) {
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartCameraTask */
  (void)argument;

#if (TEST_SELECT == 1)
  Test_Mode_Switch_Run();
  vTaskSuspend(NULL);
#elif (TEST_SELECT == 2)
  Test_Motion_Detect_Run();
  vTaskSuspend(NULL);
#elif (TEST_SELECT == 3)
  Test_Power_Manager_Run();
  vTaskSuspend(NULL);
#elif (TEST_SELECT == 4)
  Test_Net_Diag_Run();
  vTaskSuspend(NULL);
#elif (TEST_SELECT == 5)
  Test_Net_Client_Run();
  vTaskSuspend(NULL);
#elif (TEST_SELECT == 6)
  Test_V2_Integration_Run();
  vTaskSuspend(NULL);
#else
  if (Vision_Init() != 0) {
    DBG_ERROR("[Camera] Vision_Init FAIL");
    vTaskSuspend(NULL);
  }

  Vision_SetMode(VISION_MODE_JPEG);
  OV5640_Apply_Best_Settings();
  PowerMgr_Init();
  AutoExp_Init();

  PowerMode_t prev = PWR_FULL;
  uint32_t frame_id = 0;
  Camera_RequestHealthCheck();
  DBG_INFO("[Camera] event-driven pipeline started (dual-buf)");

  for (;;) {
    uint32_t camera_evt = Camera_WaitEvents();
    if (camera_evt == 0U) {
      continue;
    }

    Vision_CaptureStart();

    PowerMode_t cur = PowerMgr_GetMode();
    uint32_t jpeg_sz = 0;
    bool gray_mot = false;

    if (cur == PWR_DETECT) {
      /* DETECT 模式: 灰度帧差 + 自适应曝光, 不入队 */
      if (Vision_IsFrameReady()) {
        uint8_t *gray_buf = Vision_GetFrameBuffer();
        gray_mot = Motion_Detect(gray_buf);
        ImageQuality_t q = AutoExp_Analyze(gray_buf);
        AutoExp_Adjust(&q);
      }
    } else {
      /* FULL/LIGHT 模式: JPEG 入队给 NetTx */
      if (Vision_IsFrameReady()) {
        jpeg_sz = Vision_GetFrameSize();
        uint8_t idx = Vision_GetActiveIdx();
        FrameDesc_t fd = {
            .frame_id = frame_id++,
            .data_len = jpeg_sz,
            .p_data = Vision_GetFrameBuffer(),
            .buf_idx = idx,
        };
        Vision_SetBufState(idx, BUF_QUEUED);
        if (osMessageQueuePut(s_frame_q, &fd, 0, 0) != osOK) {
          /* 队列满: 丢帧, 恢复缓冲区 */
          Vision_SetBufState(idx, BUF_FREE);
          DBG_WARN("[Camera] Frame dropped: backpressure");
        } else {
          /* 入队成功: 切换到下一缓冲区 */
          if (!Vision_SwitchBuffer()) {
            DBG_WARN("[Camera] No free buf, next capture overwrites");
          }
        }
      }
    }

    PowerMgr_Tick(jpeg_sz, gray_mot);
    cur = PowerMgr_GetMode();

    if (cur != prev) {
      if (cur == PWR_DETECT) {
        Vision_SetMode(VISION_MODE_GRAY);
        Motion_Init(CAM_GRAY_WIDTH, CAM_GRAY_HEIGHT);
      } else if (prev == PWR_DETECT) {
        Vision_SetMode(VISION_MODE_JPEG);
        OV5640_Apply_Best_Settings();
      }
      prev = cur;
    }
  }
#endif
  /* USER CODE END StartCameraTask */
}

/* USER CODE BEGIN Header_StartNetTask */
/**
 * @brief  NetTx 任务 (消费者): 从帧队列取数据并发送
 */
/* USER CODE END Header_StartNetTask */
void StartNetTask(void *argument) {
  /* USER CODE BEGIN StartNetTask */
  (void)argument;
  Net_Client_Init();
  DBG_INFO("[NetTx] Ready, waiting for frames");

  FrameDesc_t fd;
  for (;;) {
    if (osMessageQueueGet(s_frame_q, &fd, NULL, osWaitForever) == osOK) {
      Vision_SetBufState(fd.buf_idx, BUF_SENDING);
      Net_Client_SendImage(fd.p_data, fd.data_len, fd.frame_id);
      Vision_SetBufState(fd.buf_idx, BUF_FREE);
    }
  }
  /* USER CODE END StartNetTask */
}

/* USER CODE BEGIN Application */
static bool Camera_SetEvent(uint32_t event) {
  if (s_camera_evt == NULL) {
    return false;
  }

  uint32_t flags = osEventFlagsSet(s_camera_evt, event);
  return ((flags & osFlagsError) == 0U);
}

bool Camera_TaskEventsReady(void) {
  return (s_camera_evt != NULL);
}

bool Camera_RequestCapture(void) {
  return Camera_SetEvent(CAMERA_EVT_CAPTURE);
}

bool Camera_RequestHealthCheck(void) {
  return Camera_SetEvent(CAMERA_EVT_HEALTH);
}

void Camera_NotifyFrameDoneFromISR(void) {
  if (Task_CameraHandle != NULL) {
    (void)osThreadFlagsSet(Task_CameraHandle, CAMERA_FRAME_DONE_FLAG);
  }
}

#if (TEST_SELECT == 0)
static uint32_t Camera_WaitEvents(void) {
  if (s_camera_evt == NULL) {
    osDelay(CAMERA_HEALTH_CAPTURE_MS);
    return CAMERA_EVT_HEALTH;
  }

  uint32_t flags = osEventFlagsWait(s_camera_evt, CAMERA_EVT_ANY,
                                    osFlagsWaitAny,
                                    CAMERA_HEALTH_CAPTURE_MS);
  if ((flags & osFlagsError) == 0U) {
    return (flags & CAMERA_EVT_ANY);
  }

  if (flags == (uint32_t)osFlagsErrorTimeout) {
    return CAMERA_EVT_HEALTH;
  }

  return 0U;
}
#endif

static void StartIdlePowerTask(void *argument) {
  (void)argument;
  DBG_INFO("[IdlePower] Ready");

  for (;;) {
    osDelay(IDLE_POWER_TASK_PERIOD_MS);
    __WFI();
  }
}

static void StartRadarTask(void *argument) {
  (void)argument;
  Radar_Init();
  DBG_INFO("[Radar] Ready");

  for (;;) {
    RadarSummary_t summary;
    bool capture_request = false;

    if (Radar_Poll(&summary, &capture_request)) {
      if (capture_request) {
        (void)Camera_RequestCapture();
        DBG_INFO("[Radar] capture track=%lu range=%u conf=%u",
                 summary.track_id,
                 summary.range_cm,
                 summary.confidence);
      }
    }

    osDelay(RADAR_TASK_PERIOD_MS);
  }
}
/**
 * @brief  NetRx 任务: 云端指令接收与分发
 */
#if (TEST_SELECT == 0)
static void StartControlTask(void *argument) {
  (void)argument;
  (void)Control_Manager_Init();
  Alarm_Init();
  Servo_Init();
  DBG_INFO("[Control] Ready");

  ControlCommand_t cmd;
  for (;;) {
    if (!Control_Manager_Receive(&cmd, osWaitForever)) {
      continue;
    }

    if (cmd.type == CONTROL_CMD_ALARM) {
      Alarm_Set(cmd.payload.alarm.active);
      DBG_INFO("[Control] ALARM src=%d active=%d",
               (int32_t)cmd.source,
               cmd.payload.alarm.active ? 1 : 0);
    } else if (cmd.type == CONTROL_CMD_SERVO) {
      Servo_SetAngle(cmd.payload.servo.angle_deg);
      DBG_INFO("[Control] SERVO src=%d angle=%u",
               (int32_t)cmd.source,
               cmd.payload.servo.angle_deg);
    }
  }
}

static void StartRS485Task(void *argument) {
  (void)argument;
  (void)RS485_Manager_Init();
  (void)RS485_Driver_Init();
  Modbus_RTU_Init(MODBUS_DEFAULT_SLAVE_ADDR);
  DBG_INFO("[RS485] Ready");

  uint8_t rx_buf[RS485_TASK_RX_BUF_SIZE];
  uint8_t rx_chunk[RS485_TASK_RX_BUF_SIZE];
  uint8_t tx_buf[MODBUS_RESPONSE_BUF_SIZE];
  size_t rx_len = 0U;
  for (;;) {
    uint32_t wait_ms = (rx_len == 0U) ? osWaitForever : RS485_FRAME_GAP_MS;
    size_t n = RS485_Manager_ReadRxBytes(rx_chunk,
                                         sizeof(rx_chunk),
                                         wait_ms);
    if (n > 0U) {
      if ((rx_len + n) > sizeof(rx_buf)) {
        DBG_WARN("[RS485] RX overflow, drop frame");
        rx_len = 0U;
        continue;
      }
      memcpy(&rx_buf[rx_len], rx_chunk, n);
      rx_len += n;
    } else if (rx_len == 0U) {
      continue;
    }

    for (;;) {
      size_t expected_len = Modbus_RTU_GetExpectedLength(rx_buf, rx_len);
      if ((expected_len == 0U) && (n > 0U)) {
        break;
      }

      size_t frame_len = (expected_len > 0U) ? expected_len : rx_len;
      if (rx_len < frame_len) {
        break;
      }

      size_t rsp_len = Modbus_RTU_ProcessFrame(rx_buf,
                                               frame_len,
                                               tx_buf,
                                               sizeof(tx_buf));
      if (rsp_len > 0U) {
        if (RS485_Manager_WriteTxBytes(tx_buf, rsp_len, 0U) == rsp_len) {
          (void)RS485_Driver_FlushTxStream(RS485_TX_TIMEOUT_MS);
        } else {
          DBG_WARN("[RS485] TX stream full, drop response");
        }
      }

      rx_len -= frame_len;
      if (rx_len > 0U) {
        memmove(rx_buf, &rx_buf[frame_len], rx_len);
      } else {
        break;
      }
    }
  }
}
#endif

/**
 * @brief  NetRx command receive task.
 */
static void StartNetRxTask(void *argument) {
  (void)argument;
  /* 初始化独立的接收 netconn */
  if (Net_Client_InitRx() != 0) {
    DBG_ERROR("[NetRx] InitRx FAIL");
    vTaskSuspend(NULL);
  }
  DBG_INFO("[NetRx] Listening for commands");

  IVCIS_Command_t cmd;
  for (;;) {
    /* 阻塞式接收, 无需轮询 */
    if (Net_Client_RecvCommand(&cmd)) {
      if (cmd.cmd_type == CMD_TYPE_VIOLATION) {
        (void)Control_Manager_SubmitAlarm(
            CONTROL_SRC_ETH,
            cmd.payload.violation.is_violation != 0);
        DBG_INFO("[NetRx] VIOLATION: %s (LED=%d)",
                 cmd.payload.violation.plate,
                 cmd.payload.violation.is_violation);
      } else if (cmd.cmd_type == CMD_TYPE_SERVO) {
        (void)Control_Manager_SubmitServo(CONTROL_SRC_ETH,
                                          cmd.payload.servo.angle_deg);
        DBG_INFO("[NetRx] SERVO: %u deg", cmd.payload.servo.angle_deg);
      }
    }
  }
}
/* USER CODE END Application */
