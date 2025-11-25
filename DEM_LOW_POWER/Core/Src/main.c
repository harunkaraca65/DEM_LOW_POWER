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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// Flag to indicate wakeup from RTC
volatile uint8_t wakeup_flag = 0;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    wakeup_flag = 1; // Set the flag when RTC wakes up MCU
}
void Set_RTC_Wakeup(uint32_t counter)
{
    if(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, counter, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
    {
        Error_Handler();
    }
}
/**
 * @brief  Check if MCU woke up from Standby and handle post-wakeup actions
 * @note   Clears the Standby flag, disables wakeup pin, deactivates RTC wakeup,
 *         and toggles the green LED for 2 seconds as a visual indication.
 */
void Handle_Standby_Wakeup(void)
{
    // Check if MCU woke up from STANDBY
    if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        // Clear the Standby flag to prevent immediate re-entry
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);


        // Green LED toggle to indicate wakeup from Standby
        for(int i = 0; i < 20; i++)
        {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
            HAL_Delay(100); // 100ms × 20 = 2s
        }

        // Ensure LED is off after toggling
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
    }
    else
    {
    	/* Ensure no leftover wakeup/standby flags from previous resets/power-on */
    	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF);
    	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    	__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);

    	/* Make sure RTC wakeup timer is deactivated (clean start) */
    	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

    	/* Small delay to allow LSI/LSE stabilization (if using LSE/LSI) */
    	HAL_Delay(50);

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
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  // Check if MCU woke up from STANDBY
  Handle_Standby_Wakeup();
  // ---------- RUN MODE ----------
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET); // PA15 ON
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	   // --------------------------
	    // Wait for button press to start Sleep
	    // --------------------------
	    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_RESET)
	    {
	        HAL_Delay(50); // simple debounce
	        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_RESET)
	        {
	            // ---------- SLEEP PRE-Toggle ----------
	            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
	            HAL_Delay(500);

	            for(int i=0;i<20;i++) // 2s toggle PA15
	            {
	                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
	                HAL_Delay(100);
	            }
	            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);

	            // ---------- SLEEP MODE ----------
	            wakeup_flag = 0;

	            /*
	             Enable RTC Wakeup Timer for Sleep Mode
	             - Desired Sleep duration: 2 seconds
	             - RTC Wakeup Clock Source: LSI (32 kHz)
	             - RTC_WAKEUPCLOCK_RTCCLK_DIV16 divides LSI by 16

	             Calculation:
	             WakeUp Time Base = 16 / 32kHz = 0.0005 sec
	             WakeUpCounter = Sleep Time / Time Base
	             WakeUpCounter = 2s / 0.0005s = 4000 = 0x0FA0
	            */
	            Set_RTC_Wakeup(0x0FA0); // 2s

	            HAL_SuspendTick();
	            HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	            HAL_ResumeTick();

	            // ---------- STOP PRE-Toggle ----------
	            for(int i=0;i<20;i++) // 2s toggle PD0
	            {
	                HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_0);
	                HAL_Delay(100);
	            }
	            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_RESET);

	            // ---------- STOP MODE ----------
	            wakeup_flag = 0;

	            /*
	             Enable RTC Wakeup Timer for Stop Mode
	             - Desired Stop duration: 5 seconds
	             - WakeUp Time Base = 16 / 32kHz = 0.0005 sec
	             - WakeUpCounter = 5 / 0.0005 = 10000 = 0x2710
	            */
	            Set_RTC_Wakeup(0x2710);             // Set RTC wakeup timer for 5s

	            HAL_SuspendTick();                   // Stop SysTick to prevent unwanted wakeups

	            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); // Enter STOP mode (wait for interrupt)

	            SystemClock_Config();                // Reconfigure system clock (STOP mode may change it)

	            HAL_ResumeTick();                    // Resume SysTick after waking up

	            HAL_RTCEx_DeactivateWakeUpTimer(&hrtc); // Stop the RTC wakeup timer


	            // ---------- STANDBY PRE-Toggle ----------
	            for(int i=0;i<20;i++) // 2s toggle PD1
	            {
	             HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_1);
	             HAL_Delay(100);
	            }HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);

	            // ---------- STANDBY MODE ----------
	            /*
	             - Clear previous wakeup flags to prevent immediate wakeup
	             - Enable WAKEUP pin (PA0) as a source
	             - RTC Wakeup Timer for 10 seconds

	             Calculation:
	             WakeUp Time Base = 16 / 32kHz = 0.0005 sec
	             WakeUpCounter = 10 / 0.0005 = 20000 = 0x4E20
	            */

	            // Check if MCU woke up from STANDBY

	            /* clear leftover flags just before entering Standby */
	            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF);
	            __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);

	            /* Small settle time for RTC/LSI/LSE to be stable */
	            HAL_Delay(10);

	            /* Set RTC wakeup timer and immediately enter Standby (no prints/delays between) */
	            Set_RTC_Wakeup(0x4E20); // 10s
	            HAL_PWR_EnterSTANDBYMode();


	        }
	    }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 249;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RUN_GPIO_Port, RUN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, SLEEP_Pin|STOP_Pin|STANDBY_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RUN_Pin */
  GPIO_InitStruct.Pin = RUN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RUN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SLEEP_Pin STOP_Pin STANDBY_Pin */
  GPIO_InitStruct.Pin = SLEEP_Pin|STOP_Pin|STANDBY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
