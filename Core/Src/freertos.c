/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : freertos.c
 * @brief          : 任务调度与系统逻辑核心
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
#include "Motion_Detect.h"
#include "Power_Manager.h"
#include "Vision_Pipeline.h"
#include "app_config.h"
#include "debug_config.h"
#include "ov5640.h"
#include "shared_types.h"

#if (TEST_SELECT == 1)
#include "test_mode_switch.h"
#elif (TEST_SELECT == 2)
#include "test_motion_detect.h"
#elif (TEST_SELECT == 3)
#include "test_power_manager.h"
#elif (TEST_SELECT == 4)
#include "test_net_diag.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

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
    .name = "Task_Net",
    .stack_size = 2048 * 4,
    .priority = (osPriority_t)osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

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
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_Camera */
  Task_CameraHandle =
      osThreadNew(StartCameraTask, NULL, &Task_Camera_attributes);

  /* creation of Task_Net */
  Task_NetHandle = osThreadNew(StartNetTask, NULL, &Task_Net_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartCameraTask */
/**
 * @brief  Camera 任务: 三级功耗状态机驱动
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
#else
  if (Vision_Init() != 0) {
    DBG_ERROR("[Camera] Vision_Init FAIL");
    vTaskSuspend(NULL);
  }

  Vision_SetMode(VISION_MODE_JPEG);
  OV5640_Apply_Best_Settings();
  PowerMgr_Init();

  PowerMode_t prev = PWR_FULL;
  DBG_INFO("[Camera] PowerMgr started");

  for (;;) {
    Vision_CaptureStart();

    PowerMode_t cur = PowerMgr_GetMode();
    uint32_t jpeg_sz = 0;
    bool gray_mot = false;

    if (cur == PWR_DETECT) {
      if (Vision_IsFrameReady()) {
        gray_mot = Motion_Detect(Vision_GetFrameBuffer());
      }
    } else {
      if (Vision_IsFrameReady()) {
        jpeg_sz = Vision_GetFrameSize();
        Vision_SendFrameUART();
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

    osDelay(PowerMgr_GetDelay());
  }
#endif
  /* USER CODE END StartCameraTask */
}

/* USER CODE BEGIN Header_StartNetTask */
/**
 * @brief  Net 任务: LwIP 协议栈处理
 */
/* USER CODE END Header_StartNetTask */
void StartNetTask(void *argument) {
  /* USER CODE BEGIN StartNetTask */
  (void)argument;
  DBG_INFO("[Net] LwIP OK");

  for (;;) {
    /* TODO: JPEG -> UDP */
    osDelay(10);
  }
  /* USER CODE END StartNetTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
