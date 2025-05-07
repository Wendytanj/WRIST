/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "app_tcpp.h"
#include "usbpd.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "string.h"
#include "stdlib.h"
#include <math.h>
#include <stdbool.h>
#include "Haptic_Driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PULSES_PER_REV 17920
#define PULSES_EACH_MOT_REV 16
#define IDLE_STATE 0
#define INIT_STATE 1
#define MODEL_STATE 2
#define MODELED_STATE 3
#define RETURN_STATE 4
#define READY_STATE 5
#define FUNCTION_STATE 6
#define TAKEOFF_STATE 7
#define O2REF_TIME 1000
#define RX_BUFFER_SIZE 128
// did we use this???
#define MAX_WAVE_SIZE 128
#define NUM_DRIVERS 3
#define WAVEFORM_CMD_MAX_LENGTH 512
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim1;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
volatile uint8_t rxBuffer[RX_BUFFER_SIZE];
volatile uint16_t rxBufferIndex;
volatile uint8_t rxLineReady;
uint32_t encoderPos = 0;
float ctrl_coeff = 1;
int rotate_counter = 0;
int state = IDLE_STATE;
uint32_t time_start = 0;
uint32_t init_interval = 0;
uint32_t O2_interval = 0;
uint32_t rev_start = 0;
uint32_t rev_end = 0;
uint32_t init_loc = 0;
uint32_t final_loc = 0;

QueueHandle_t waveformQueue;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_UCPD1_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef struct {
    Haptic_Driver driver;      // The haptic driver instance.
    I2C_HandleTypeDef* i2c;     // I2C bus handle for this device.
    uint8_t address;           // The device's I2C address.
} Haptic_Device;

#define NUM_HAPTIC_DEVICES 5

Haptic_Device hapticDevices[NUM_HAPTIC_DEVICES];

void trim(char *str)
{
    // Trim trailing whitespace (including '\r' and '\n')
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\r' || str[len-1] == '\n' || str[len-1] == ' '))
    {
        str[len-1] = '\0';
        len--;
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_UCPD1_Init();
  MX_LPUART1_UART_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TCPP_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* USBPD initialisation ---------------------------------*/
  MX_USBPD_Init();

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
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityHigh, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00900D1E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_DISABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 1) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Fast mode Plus enable
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00900D1E;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_DISABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 1) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Fast mode Plus enable
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C2);
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  LL_LPUART_InitTypeDef LPUART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LPUART1);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**LPUART1 GPIO Configuration
  PA2   ------> LPUART1_TX
  PA3   ------> LPUART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* LPUART1 DMA Init */

  /* LPUART1_TX Init */
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMAMUX_REQ_LPUART1_TX);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);

  /* LPUART1 interrupt Init */
  NVIC_SetPriority(USART3_4_5_6_LPUART1_IRQn, 3);
  NVIC_EnableIRQ(USART3_4_5_6_LPUART1_IRQn);

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  LPUART_InitStruct.PrescalerValue = LL_LPUART_PRESCALER_DIV1;
  LPUART_InitStruct.BaudRate = 912600;
  LPUART_InitStruct.DataWidth = LL_LPUART_DATAWIDTH_7B;
  LPUART_InitStruct.StopBits = LL_LPUART_STOPBITS_1;
  LPUART_InitStruct.Parity = LL_LPUART_PARITY_NONE;
  LPUART_InitStruct.TransferDirection = LL_LPUART_DIRECTION_TX_RX;
  LPUART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
  LL_LPUART_Init(LPUART1, &LPUART_InitStruct);
  LL_LPUART_SetTXFIFOThreshold(LPUART1, LL_LPUART_FIFOTHRESHOLD_1_8);
  LL_LPUART_SetRXFIFOThreshold(LPUART1, LL_LPUART_FIFOTHRESHOLD_1_8);
  LL_LPUART_DisableFIFO(LPUART1);

  /* USER CODE BEGIN WKUPType LPUART1 */

  /* USER CODE END WKUPType LPUART1 */

  LL_LPUART_Enable(LPUART1);

  /* Polling LPUART1 initialisation */
  while((!(LL_LPUART_IsActiveFlag_TEACK(LPUART1))) || (!(LL_LPUART_IsActiveFlag_REACK(LPUART1))))
  {
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 63;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 255;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief UCPD1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UCPD1_Init(void)
{

  /* USER CODE BEGIN UCPD1_Init 0 */

  /* USER CODE END UCPD1_Init 0 */

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UCPD1);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**UCPD1 GPIO Configuration
  PB15   ------> UCPD1_CC2
  PA8   ------> UCPD1_CC1
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* UCPD1 DMA Init */

  /* UCPD1_RX Init */
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_4, LL_DMAMUX_REQ_UCPD1_RX);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_4, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MDATAALIGN_BYTE);

  /* UCPD1_TX Init */
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMAMUX_REQ_UCPD1_TX);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MDATAALIGN_BYTE);

  /* UCPD1 interrupt Init */
  NVIC_SetPriority(USB_UCPD1_2_IRQn, 3);
  NVIC_EnableIRQ(USB_UCPD1_2_IRQn);

  /* USER CODE BEGIN UCPD1_Init 1 */

  /* USER CODE END UCPD1_Init 1 */
  /* USER CODE BEGIN UCPD1_Init 2 */

  /* USER CODE END UCPD1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_3_IRQn interrupt configuration */
  NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3);
  NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
  /* DMA1_Ch4_7_DMA2_Ch1_5_DMAMUX1_OVR_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Ch4_7_DMA2_Ch1_5_DMAMUX1_OVR_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMA2_Ch1_5_DMAMUX1_OVR_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PWR_TCPP_GPIO_Port, PWR_TCPP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : Encoder_B_Pin */
  GPIO_InitStruct.Pin = Encoder_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Encoder_B_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Encoder_A_Pin */
  GPIO_InitStruct.Pin = Encoder_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Encoder_A_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PWR_TCPP_Pin */
  GPIO_InitStruct.Pin = PWR_TCPP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWR_TCPP_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
	if (GPIO_Pin == Encoder_A_Pin){
		GPIO_PinState b_val = HAL_GPIO_ReadPin(Encoder_B_GPIO_Port, Encoder_B_Pin);

		if (b_val == GPIO_PIN_RESET){
			encoderPos--;
		} else {
			encoderPos++;
		}
		rotate_counter++;
		if (rotate_counter == PULSES_EACH_MOT_REV){
		    rotate_counter = 0;
		    if (state == MODEL_STATE){
		      // first rev completed record how long it took
		      rev_start = HAL_GetTick();
		      init_interval = rev_start - time_start;
		      state = MODELED_STATE;
		    }
//		    } else if (state == TAKEOFF_STATE){
//		      // keep untight till we have same speed as the original speed
//		      rev_end = HAL_GetTick();
//		      uint32_t difference = rev_end - rev_start;
//		      if (abs(difference - init_interval) < 10) {
//		        // reach backup place, go to ready state
//		        state = IDLE_STATE;
//		        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
//		        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
//		      }
//		      rev_start = rev_end;
//		    }
		  }
	}
}

void rotateAngle(float targetAngle, int clockwise, int speed) {

  uint64_t targetPulses = (uint64_t) round((targetAngle * PULSES_PER_REV) / 360.0);

  __disable_irq();
  uint64_t startCount = encoderPos;
  __enable_irq();
  uint64_t delta;
  uint64_t error;
  uint64_t currentCount;
  uint32_t time_beg = HAL_GetTick();
  while (1) {
	__disable_irq();
	currentCount = encoderPos;
    __enable_irq();

    delta = abs(currentCount - startCount);
    error = targetPulses - delta;

    if (error <= 0 || ((HAL_GetTick() - time_beg) >= 1000)) {
    	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 255);
    	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 255);
      break;
    }

    if (clockwise) {
    	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speed);
    	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    } else {
    	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, speed);
    }
  }
}

void initHapticDevices(void) {
    // I2C1 devices:
    Haptic_Driver_init(&hapticDevices[0].driver, 0x48);
    hapticDevices[0].i2c = &hi2c1;
    hapticDevices[0].address = 0x48;

    Haptic_Driver_init(&hapticDevices[1].driver, 0x49);
    hapticDevices[1].i2c = &hi2c1;
    hapticDevices[1].address = 0x49;

    Haptic_Driver_init(&hapticDevices[2].driver, 0x4A);
    hapticDevices[2].i2c = &hi2c1;
    hapticDevices[2].address = 0x4A;

    // I2C2 devices:
    Haptic_Driver_init(&hapticDevices[3].driver, 0x48);
    hapticDevices[3].i2c = &hi2c2;
    hapticDevices[3].address = 0x48;

    Haptic_Driver_init(&hapticDevices[4].driver, 0x49);
    hapticDevices[4].i2c = &hi2c2;
    hapticDevices[4].address = 0x49;
}

Haptic_Driver* getDriverByID(int id)
{
    // Device IDs are expected to be 1-indexed.
    if (id < 1 || id > NUM_HAPTIC_DEVICES)
        return NULL;
    return &hapticDevices[id - 1].driver;
}


void parseWCommand(const char *cmd)
{
    // Make a mutable copy of the incoming command string.
    char buffer[128];
    strncpy(buffer, cmd, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    // Tokenize the command (expected format: "W addr_indicator ampl")
    char *tokens[3];
    int tokenCount = 0;
    char *token = strtok(buffer, " ");
    while (token != NULL && tokenCount < 3)
    {
        tokens[tokenCount++] = token;
        token = strtok(NULL, " ");
    }

    if (tokenCount < 3)
    {
        char errMsg[] = "Error: W command format incorrect\r\n";
        CDC_Transmit_FS((uint8_t*)errMsg, strlen(errMsg));
        return;
    }

    // tokens[0] should be "W" (command identifier)

    // Parse device address indicator from tokens[1]
    int addrIndicator = atoi(tokens[1]);

    // Parse amplitude from tokens[2].
    // Here, we assume the amplitude is sent in hexadecimal.
    // Change the base (last parameter) to 10 if using decimal.
    uint8_t amplitude = (uint8_t)strtol(tokens[2], NULL, 16);

    // Obtain the driver pointer from the provided address indicator.
    Haptic_Driver *drv = getDriverByID(addrIndicator);
    if (drv != NULL)
    {
        Haptic_Driver_setVibrate(drv, amplitude);

        // Optionally send an acknowledgement back.
        char ackMsg[64];
        sprintf(ackMsg, "Device %d set to amplitude 0x%02X\r\n", addrIndicator, amplitude);
        CDC_Transmit_FS((uint8_t*)ackMsg, strlen(ackMsg));
    }
    else
    {
        char errMsg[] = "Error: Invalid device indicator\r\n";
        CDC_Transmit_FS((uint8_t*)errMsg, strlen(errMsg));
    }
}

int HapticDriverStart1(Haptic_Driver *testDrv){
	if (testDrv == NULL || !Haptic_Driver_begin(testDrv, &hi2c1))
	  {
		  return 1;
	  }
	  else
	  {
		  if (!Haptic_Driver_defaultMotor(testDrv))
		  {
			  return 2;
		  }
		  Haptic_Driver_enableFreqTrack(testDrv, false);
		  Haptic_Driver_setOperationMode(testDrv, DRO_MODE);
		  return 0;
	  }
}

int HapticDriverStart2(Haptic_Driver *testDrv){
	if (testDrv == NULL || !Haptic_Driver_begin(testDrv, &hi2c2))
	  {
		  return 1;
	  }
	  else
	  {
		  if (!Haptic_Driver_defaultMotor(testDrv))
		  {
			  return 2;
		  }
		  Haptic_Driver_enableFreqTrack(testDrv, false);
		  Haptic_Driver_setOperationMode(testDrv, DRO_MODE);
		  return 0;
	  }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_Device */
  MX_USB_Device_Init();
  /* USER CODE BEGIN 5 */
  osDelay(100);
  char cmd;
  float angle = 0.0;
  int amp = 250;
  int base = 60;
  int level = 0;
  int diff = amp - base;
  int mok = 1;
  int prev_p = -1;
  char charData[64];
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1); // for PB13, if TIM1_CH1
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2); // for PB14, if TIM1_CH2
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); // PB13
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); // PB14

  initHapticDevices();
  Haptic_Driver *HapDrv1 = getDriverByID(1);
  Haptic_Driver *HapDrv2 = getDriverByID(2);
  Haptic_Driver *HapDrv3 = getDriverByID(3);
  Haptic_Driver *HapDrv4 = getDriverByID(4);
  Haptic_Driver *HapDrv5 = getDriverByID(5);

  if (HapticDriverStart1(HapDrv1)) {
	  CDC_Transmit_FS((uint8_t*)"Error: Could not set default settings on device1\r\n",
	  						  strlen("Error: Could not set default settings on device1\r\n"));
  }
  if (HapticDriverStart1(HapDrv2)) {
  	  CDC_Transmit_FS((uint8_t*)"Error: Could not set default settings on device2\r\n",
  	  						  strlen("Error: Could not set default settings on device2\r\n"));
    }
  if (HapticDriverStart1(HapDrv3)) {
  	  CDC_Transmit_FS((uint8_t*)"Error: Could not set default settings on device3\r\n",
  	  						  strlen("Error: Could not set default settings on device3\r\n"));
    }
  if (HapticDriverStart2(HapDrv4)) {
  	  CDC_Transmit_FS((uint8_t*)"Error: Could not set default settings on device4\r\n",
  	  						  strlen("Error: Could not set default settings on device4\r\n"));
    }
  if (HapticDriverStart2(HapDrv5)) {
  	  CDC_Transmit_FS((uint8_t*)"Error: Could not set default settings on device5\r\n",
  	  						  strlen("Error: Could not set default settings on device5\r\n"));
    }

  /* Infinite loop */
  for(;;)
  {
	  if (rxLineReady){
		  // Something got read, string processing
		  // ASSUME!!!!!! CMD IS LIKE A LETTER + A NUMBER
		  //EG: S 255, or A 17.5!!!
		  char trimmed[RX_BUFFER_SIZE];
		  strcpy(trimmed, (char *)rxBuffer);
		  trim(trimmed);
		  cmd = (char) trimmed[0];
		  if (cmd == 'S'){
			  // this is set new amp
			  amp = atoi((char*) &trimmed[2]);
		  } else if (cmd == 'A') {
			  angle = atof((char*) &trimmed[2]);
			  rotateAngle(angle, 1, amp);
		  } else if (cmd == 'B'){
			  angle = atof((char*) &trimmed[2]);
			  rotateAngle(angle, 0, amp);
		  } else if (cmd == 'R'){
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, amp); // PB13
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); // PB14
		  } else if (cmd == 'L'){
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); // PB13
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, amp); // PB14
		  } else if (cmd == 'X'){
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 255); // PB13
			  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 255); // PB14
		  } else if (cmd == 'M'){
			  //record & model the wrist begins, will run as the IRS goes
			  if (mok){
			  		state = INIT_STATE;
			  		mok = 0;
			  }
		  } else if (cmd == 'T'){
		      //record & model the wrist begins, will run as the IRS goes
				  state = TAKEOFF_STATE;
				  mok = 1;
				  prev_p = -1;
		  } else if (cmd == 'W') {
              parseWCommand(trimmed);
          } else if (cmd == 'V') {
        	  uint8_t amplitude = (uint8_t)strtol((char*) &trimmed[2], NULL, 16);
        	  Haptic_Driver_setVibrate(HapDrv1, amplitude);
        	  Haptic_Driver_setVibrate(HapDrv2, amplitude);
        	  Haptic_Driver_setVibrate(HapDrv3, amplitude);
        	  Haptic_Driver_setVibrate(HapDrv4, amplitude);
        	  Haptic_Driver_setVibrate(HapDrv5, amplitude);
          } else if (cmd == 'P') {
        	  diff = amp - base;
        	  level = atoi((char*) &trimmed[2]);
        	  if (level >= prev_p){ // increase in amp, so no action taken
        		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, base + (diff * level / 10)); // PB13
        		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); // PB14
        	  } else { // decrease in amp, so back up a bit
        		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
        		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, amp);
        		  HAL_Delay(30);
        		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, base + (diff * level / 10)); // PB13
        		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); // PB14
        	  }
        	  prev_p = level;
          }
		  sprintf(charData, "%s\n", trimmed);
		  CDC_Transmit_FS((uint8_t *) charData, strlen(charData));
		  rxLineReady = 0;
		  rxBufferIndex = 0;
	  }
	  if (state == INIT_STATE){
		  // begin the modeling process, assume cw is tighting, ccw is untighting
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, amp);
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
		  // make the thing rotate for 1 to find how fast is it, also record the start time
		  time_start = HAL_GetTick();
		  rotate_counter = 0;
		  state = MODEL_STATE;
//		  __disable_irq();
//		  init_loc = encoderPos;
//		  __enable_irq();
//	  } else if (state == MODELED_STATE) {
//		  // check how long does this case compare to others, then save the coefficient!
//		  ctrl_coeff = (float) O2_interval / O2REF_TIME;
//		  state = RETURN_STATE;
//		  __disable_irq();
//		  final_loc = encoderPos;
//		  __enable_irq();
//		  // goes back a bit....
//		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
//		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, amp);
//	  } else if (state == READY_STATE) {
//		  // setup completed!!!
//		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 255);
//		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 255);
//		  state = FUNCTION_STATE;
	  } else if (state == TAKEOFF_STATE) {
		  // return to the init setup for user to take off the device
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, amp);
//		  for (int i; i <10000;i++){
//
//		  }
		  HAL_Delay(100);
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
		  state = FUNCTION_STATE;
	  }
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
