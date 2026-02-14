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
#include "Vision_Pipeline.h"
#include "app_config.h"
#include "debug_config.h"
#include "shared_types.h"

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
/* Definitions for Task_AI */
osThreadId_t Task_AIHandle;
const osThreadAttr_t Task_AI_attributes = {
    .name = "Task_AI",
    .stack_size = 4096 * 4,
    .priority = (osPriority_t)osPriorityNormal,
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
void StartAITask(void *argument);
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

  /* creation of Task_AI */
  Task_AIHandle = osThreadNew(StartAITask, NULL, &Task_AI_attributes);

  /* creation of Task_Net */
  Task_NetHandle = osThreadNew(StartNetTask, NULL, &Task_Net_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  extern void Test_Net_Client_Run(void);
  // Test_Net_Client_Run(); /* Uncomment to run TDD */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartCameraTask */
/**
 * @brief  Task_Camera: Vision capture + Motion detection
 */
/* USER CODE END Header_StartCameraTask */
void StartCameraTask(void *argument) {
  /* init code for LWIP */
  /* USER CODE BEGIN StartCameraTask */
  (void)argument;
  DBG_INFO("[Camera] Init...");

  if (Vision_Init() != 0) {
    DBG_ERROR("[Camera] Vision_Init FAILED, task suspended");
    vTaskSuspend(NULL);
  }

  Vision_SetMode(VISION_MODE_GRAY);
  Motion_Init(CAM_GRAY_WIDTH, CAM_GRAY_HEIGHT);

  /* Discard first 3 frames for sensor stabilization */
  for (uint8_t w = 0; w < 3; w++) {
    Vision_CaptureStart();
  }
  DBG_INFO("[Camera] Running: Gray 160x120 motion detect");

  for (;;) {
    Vision_CaptureStart();

    if (Vision_IsFrameReady()) {
      uint8_t *frame = Vision_GetFrameBuffer();
      bool motion = Motion_Detect(frame);
      uint32_t diff = Motion_GetDiff();

      if (motion) {
        DBG_INFO("[Camera] Motion! Diff=%lu", diff);
        /* TODO: switch JPEG, capture, signal Net task */
      }
    }

    osDelay(100);
  }
  /* USER CODE END StartCameraTask */
}

/* USER CODE BEGIN Header_StartAITask */
/**
 * @brief  Task_AI: Reserved stub (AI runs on cloud)
 */
/* USER CODE END Header_StartAITask */
void StartAITask(void *argument) {
  /* USER CODE BEGIN StartAITask */
  (void)argument;
  /* AI inference runs on cloud, this task is a CubeMX stub */
  vTaskSuspend(NULL);
  /* USER CODE END StartAITask */
}

/* USER CODE BEGIN Header_StartNetTask */
/**
 * @brief  Task_Net: LwIP protocol stack processing
 */
/* USER CODE END Header_StartNetTask */
void StartNetTask(void *argument) {
  /* USER CODE BEGIN StartNetTask */
  (void)argument;
  MX_LWIP_Init();
  DBG_INFO("[Net] LwIP initialized");

  for (;;) {
    /* TODO: check for JPEG frames to send via UDP */
    osDelay(10);
  }
  /* USER CODE END StartNetTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
