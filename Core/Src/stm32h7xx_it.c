/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32h7xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_it.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern ETH_HandleTypeDef heth;
extern DMA_HandleTypeDef hdma_dcmi;
extern DCMI_HandleTypeDef hdcmi;
extern DMA2D_HandleTypeDef hdma2d;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim6;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1) {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
  /* USER CODE BEGIN HardFault_IRQn 0 */
  /* Extract the stacked PC/LR from the exception frame */
  volatile uint32_t *sp;
  __asm volatile("TST LR, #4\n"
                 "ITE EQ\n"
                 "MRSEQ %0, MSP\n"
                 "MRSNE %0, PSP\n"
                 : "=r"(sp));

  uint32_t stk_pc = sp[6];
  uint32_t stk_lr = sp[5];
  uint32_t cfsr = *(volatile uint32_t *)0xE000ED28;
  const char hx[] = "0123456789ABCDEF";
  uint32_t vals[3];
  const char *tags[3];
  vals[0] = stk_pc;
  vals[1] = stk_lr;
  vals[2] = cfsr;
  tags[0] = "\r\nHF! PC=0x";
  tags[1] = " LR=0x";
  tags[2] = "\r\nCFSR=0x";
  for (int t = 0; t < 3; t++) {
    const char *s = tags[t];
    while (*s) {
      while (!(USART3->ISR & USART_ISR_TXE_TXFNF))
        ;
      USART3->TDR = *s++;
    }
    for (int i = 28; i >= 0; i -= 4) {
      while (!(USART3->ISR & USART_ISR_TXE_TXFNF))
        ;
      USART3->TDR = hx[(vals[t] >> i) & 0xF];
    }
  }
  while (!(USART3->ISR & USART_ISR_TXE_TXFNF))
    ;
  USART3->TDR = '\r';
  while (!(USART3->ISR & USART_ISR_TXE_TXFNF))
    ;
  USART3->TDR = '\n';
  while (1) {
    for (volatile uint32_t i = 0; i < 2000000; i++)
      ;
    USART3->TDR = 'X';
  }
  /* USER CODE END HardFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void) {
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles USART2 global interrupt.
 */
void USART2_IRQHandler(void) {
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
 * @brief This function handles USART3 global interrupt.
 */
void USART3_IRQHandler(void) {
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/**
 * @brief This function handles TIM6 global interrupt, DAC1_CH1 and DAC1_CH2
 * underrun error interrupts.
 */
void TIM6_DAC_IRQHandler(void) {
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
 * @brief This function handles DMA2 stream0 global interrupt.
 */
void DMA2_Stream0_IRQHandler(void) {
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */

  /* USER CODE END DMA2_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_dcmi);
  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */

  /* USER CODE END DMA2_Stream0_IRQn 1 */
}

/**
 * @brief This function handles Ethernet global interrupt.
 */
void ETH_IRQHandler(void) {
  /* USER CODE BEGIN ETH_IRQn 0 */

  /* USER CODE END ETH_IRQn 0 */
  HAL_ETH_IRQHandler(&heth);
  /* USER CODE BEGIN ETH_IRQn 1 */

  /* USER CODE END ETH_IRQn 1 */
}

/**
 * @brief This function handles DCMI global interrupt.
 */
void DCMI_IRQHandler(void) {
  /* USER CODE BEGIN DCMI_IRQn 0 */

  /* USER CODE END DCMI_IRQn 0 */
  HAL_DCMI_IRQHandler(&hdcmi);
  /* USER CODE BEGIN DCMI_IRQn 1 */

  /* USER CODE END DCMI_IRQn 1 */
}

/**
 * @brief This function handles DMA2D global interrupt.
 */
void DMA2D_IRQHandler(void) {
  /* USER CODE BEGIN DMA2D_IRQn 0 */

  /* USER CODE END DMA2D_IRQn 0 */
  HAL_DMA2D_IRQHandler(&hdma2d);
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/**
 * @brief This function handles MDMA global interrupt.
 */
void MDMA_IRQHandler(void) {
  /* USER CODE BEGIN MDMA_IRQn 0 */

  /* USER CODE END MDMA_IRQn 0 */
  /* USER CODE BEGIN MDMA_IRQn 1 */

  /* USER CODE END MDMA_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
