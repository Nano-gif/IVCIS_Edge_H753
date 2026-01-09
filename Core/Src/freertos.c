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
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ov5640.h"
#include "Vision_Pipeline.h"
#include "app_config.h"
#include "shared_types.h"
#include "dma.h"
#include "lwip/netif.h"    // 提供 netif_is_link_up 和 netif_ip4_addr
#include "lwip/ip_addr.h"   // 提供 ip4addr_ntoa
#include "Vision_Pipeline.h" // 以后我们要在这里调用视觉接口
#include "Net_Client.h"
#include <string.h>
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
// 引用 main.c 中定义的缓冲区用于调试检查
osSemaphoreId_t Sem_AI_Handle;  // Camera -> AI 的信号
osSemaphoreId_t Sem_Net_Handle; // AI -> Net 的信号
extern DMA_HandleTypeDef hdma_dcmi;
extern DCMI_HandleTypeDef hdcmi;
extern struct netif gnetif;
extern ETH_HandleTypeDef heth;
extern ETH_DMADescTypeDef DMATxDscrTab[];
/* USER CODE END Variables */
/* Definitions for Task_Camera */
osThreadId_t Task_CameraHandle;
const osThreadAttr_t Task_Camera_attributes = {
  .name = "Task_Camera",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task_AI */
osThreadId_t Task_AIHandle;
const osThreadAttr_t Task_AI_attributes = {
  .name = "Task_AI",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Net */
osThreadId_t Task_NetHandle;
const osThreadAttr_t Task_Net_attributes = {
  .name = "Task_Net",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityLow,
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
  Task_CameraHandle = osThreadNew(StartCameraTask, NULL, &Task_Camera_attributes);

  /* creation of Task_AI */
  Task_AIHandle = osThreadNew(StartAITask, NULL, &Task_AI_attributes);

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
  * @brief  Function implementing the Task_Camera thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartCameraTask */
void StartCameraTask(void *argument)
{
  /* USER CODE BEGIN StartCameraTask */
	  printf("[SYS] Camera Task Started.\r\n");
  /* Infinite loop */
  for(;;)
  {
    osDelay(100);
  }
  /* USER CODE END StartCameraTask */
}

/* USER CODE BEGIN Header_StartAITask */
/**
* @brief Function implementing the Task_AI thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAITask */
void StartAITask(void *argument)
{
  /* USER CODE BEGIN StartAITask */
	  /* Infinite loop */
	  for(;;)
	  {
	    osDelay(100);
	  }
  /* USER CODE END StartAITask */
}

/* USER CODE BEGIN Header_StartNetTask */
/**
* @brief Function implementing the Task_Net thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartNetTask */
void StartNetTask(void *argument)
{
  /* USER CODE BEGIN StartNetTask */
	  MX_LWIP_Init();
	  Net_Client_Init();

	  for(;;)
	  {
	    /* 检查 JPEG 编码完成标志 (由中断回调设置) */
	    if (jpeg_encode_complete) {
	      jpeg_encode_complete = 0;
	      Net_Client_SendImage(JPEG_Out_Buf, last_jpeg_actual_size, full_transfer_count);
	    }
	    osDelay(10);
	  }
  /* USER CODE END StartNetTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

