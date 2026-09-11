/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "led.h"
#include "beep.h"
#include "key.h"
#include "fan.h"
#include "light.h"
#include "temp_ctrl.h"
#include "uart.h"
#include "OLED.h"
#include "clock.h"
#include "delay.h"
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
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ============================================================
 *  闪烁基准：LED0 每 BLINK_RAW_LOOP 次主循环翻转一次
 *  ------------------------------------------------------------
 *  这里是「固定循环次数」，不是固定时间 —— 时钟越慢，同样次数花的时间越长，
 *  所以三种模式的闪烁速度会明显不同（8MHz 比 168MHz 慢 21 倍）。
 *
 *  故意不用 HAL_Delay()/Delay_Ms()：它们在切时钟后会被重新换算，
 *  三种模式永远一样快，就看不出差别了。
 *
 *  实测半周期会显示在 OLED 的 BLINK 那一行，嫌快嫌慢就改这个数字，
 *  半周期与它成正比。
 * ============================================================ */
#define BLINK_RAW_LOOP   50000U

/* 闪烁节拍状态（放文件作用域，UI_ShowClock 要读实测周期） */
static uint32_t s_blink_cnt     = 0;   /* 主循环计数 */
static uint32_t s_blink_t0      = 0;   /* 上次翻转时刻(ms) */
static uint32_t s_blink_half_ms = 0;   /* 实测半周期(ms) */

/**
 * @brief  OLED 刷新：当前时钟模式 + 频率 + 闪烁实测周期 + 运行时间
 * @note   SYSCLK / MCO1 全部从 RCC 寄存器反算，不写死
 */
static void UI_ShowClock(void)
{
  uint32_t khz;
  uint32_t sec = HAL_GetTick() / 1000U;

  OLED_ShowString(0, 0, "CLK TEST", OLED_8X16);

  OLED_ShowString(0, 16, "MODE  : ", OLED_6X8);
  OLED_ShowString(48, 16, Clock_GetModeName(), OLED_6X8);

  /* SYSCLK：直接问 HAL 要，它从 RCC->CFGR 反算 */
  OLED_ShowString(0, 24, "SYSCLK: ", OLED_6X8);
  OLED_ShowNum(48, 24, HAL_RCC_GetSysClockFreq() / 1000000U, 3, OLED_6X8);
  OLED_ShowString(66, 24, "MHz", OLED_6X8);

  /* MCO1：源+分频都从 RCC->CFGR 读回来，换源后自动跟着变 */
  khz = Clock_GetMco1Freq() / 1000U;
  OLED_ShowString(0, 32, "MCO1  : ", OLED_6X8);
  OLED_ShowNum(48, 32, khz / 1000U, 3, OLED_6X8);
  OLED_ShowChar(66, 32, '.', OLED_6X8);
  OLED_ShowNum(72, 32, (khz % 1000U) / 100U, 1, OLED_6X8);
  OLED_ShowString(78, 32, "MHz", OLED_6X8);

  /* BLINK：实测半周期 —— 时钟一慢它就变大，这就是要看的现象 */
  OLED_ShowString(0, 40, "BLINK : ", OLED_6X8);
  OLED_ShowNum(48, 40, s_blink_half_ms, 4, OLED_6X8);
  OLED_ShowString(72, 40, "ms", OLED_6X8);

  /* RUN：HAL_GetTick() 走 SysTick，切时钟后 HAL 会重算重装载值，
     三种模式下走时一样快 —— 正好和 BLINK 的差别形成对照 */
  OLED_ShowString(0, 48, "RUN   : ", OLED_6X8);
  OLED_ShowNum(48, 48, (sec / 60U) % 100U, 2, OLED_6X8);
  OLED_ShowChar(60, 48, ':', OLED_6X8);
  OLED_ShowNum(66, 48, sec % 60U, 2, OLED_6X8);
  OLED_ShowChar(78, 48, 's', OLED_6X8);
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
  MX_USART3_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  LED_Init();
  Key_Init();
  Delay_Init(); /* DWT 延时模块（备用，主循环现在用固定循环延时） */
  Clock_Init(); /* 上电默认 HSE PLL 168MHz，PA8(MCO1) 输出 33.6MHz */
  // Beep_Init();
  // Fan_Init();
  // Light_Init();
  // TempCtrl_Init();
  // UART3_Init();
  OLED_Init();
  OLED_Update();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  static uint32_t last_sec = 0;

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* 按键每轮都采样，不用等 LED 那一拍 —— 任何时钟模式下都不会漏按 */
    if (Key_GetNum() == KEY0_VAL)
    {
      static uint32_t last_press = 0;
      uint32_t now = HAL_GetTick();

      /* 软件消抖：循环现在跑到微秒级，机械抖动会产生多个边沿 */
      if ((now - last_press) > 150U)
      {
        last_press = now;
        Clock_Toggle();        /* 168MHz -> 16MHz -> 8MHz -> 168MHz ... */
        UI_ShowClock();
        OLED_Update();
      }
    }

    /* LED0(PF9)：固定循环次数延时 —— 时钟越慢闪得越慢，这就是要看的现象 */
    if (++s_blink_cnt >= BLINK_RAW_LOOP)
    {
      uint32_t now = HAL_GetTick();

      s_blink_cnt     = 0;
      s_blink_half_ms = now - s_blink_t0;   /* 实测半周期 */
      s_blink_t0      = now;
      LED_Toggle(LED0);
    }

    /* 每秒刷一次屏（RUN 走秒 + BLINK 显示实测值） */
    if ((HAL_GetTick() / 1000U) != last_sec)
    {
      last_sec = HAL_GetTick() / 1000U;
      UI_ShowClock();
      OLED_Update();
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief TIM6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 41;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 1999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */
}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

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
