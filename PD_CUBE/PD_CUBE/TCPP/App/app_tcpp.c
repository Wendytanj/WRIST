/**
 ******************************************************************************
 * @file    STMicroelectronics.X-CUBE-TCPP.1.0.0
 * @brief   App Source application C file
 ******************************************************************************
 * @attention
 *
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "app_tcpp.h"

#if defined (_RTOS)
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "usbpd.h"

#ifdef _TRACE
#include "tracer_emb.h"
#endif /*_TRACE*/

#ifdef _GUI_INTERFACE
#include "gui_api.h"
#endif /*_GUI_INTERFACE*/

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* Private function prototypes -----------------------------------------------*/
void TCPP01_GPIO_Init(void);
void USBPD_ADC1_Init(void);

void MX_TCPP_Init(void)
{
  TCPP01_GPIO_Init();
  USBPD_ADC1_Init();

  HAL_ADCEx_Calibration_Start(&hadc1);

  HAL_ADC_Start(&hadc1);
}

void MX_TCPP_Process(void)
{

}

/**
  * @brief  Initializes the TCPP01 interface : DB and VCC GPIO
  */
void TCPP01_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
  /*Configure GPIO pin : VCC */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
  /*Configure GPIO pin : DB */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
void USBPD_ADC1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /* Initializes the peripherals clocks */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  /* Peripheral clock enable */
  __HAL_RCC_ADC_CLK_ENABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();
  /* ADC1 GPIO Configuration */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* ADC1 init */
  ADC_ChannelConfTypeDef sConfig = {0};
  /* Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)*/
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  HAL_ADC_Init(&hadc1);

  /** Configure Regular Channel*/
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
  }

}

