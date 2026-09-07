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
#include "oled.h"
#include "fan.h"
#include "buzzer.h"
#include "led.h"
#include "key.h"
#include "light.h"
#include "uart_cmd.h"
#include "servo.h"
#include "image_data.h"

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  /* USER CODE BEGIN 2 */
  /* ============================================================
   * 引脚占用说明
   * ============================================================
   * PA0  - LED1（低电平点亮）
   * PA1  - LED2（低电平点亮）
   * PA2  - LED3（低电平点亮）
   * PA3  - 按键1（按下=LOW，接GND）
   * PA4  - 按键2（按下=LOW，接GND）
   * PA5  - 按键3（按下=LOW，接GND）
   * PA6  - 蜂鸣器（低电平触发）
   * PA9  - USART1_RXD（直连串口TXD）
   * PA10 - USART1_TXD（直连串口RXD）
   * PB3  - 舵机（软件PWM）
   * PB8  - OLED SCL（软件I2C）
   * PB9  - OLED SDA（软件I2C）
   * PB12 - 风扇 INB（L9110H）
   * PB13 - 风扇 IA（L9110H）
   * PB14 - 光敏传感器（数字输入）
   * PC13 - 贴片灯（低电平点亮）
   * ============================================================
   */
  Fan_Init();
  Buzzer_Init();
  LED_Init();
  OLED_Init();
  OLED_Clear();
  OLED_Update();
  Key_Init();
  Light_Init();
  UART_CMD_Init(); /* 串口命令解析：PA10=TX PA9=RX */
  Servo_Init();    /* 舵机：PA7 软件PWM */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  int mode = 10;
  int last_mode = -1; /* 记录上一次的 mode，用于检测模式切换 */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* Qt 通过串口设置的目标模式 */
    if (UART_CMD_GetTargetMode() > 0)
    {
      mode = UART_CMD_GetTargetMode();
      UART_CMD_ClearTargetMode();
    }

    /* 同步当前模式到 uart_cmd，用于 STATUS 查询 */
    UART_CMD_SetMode((uint8_t)mode);

    /* 只在模式切换时停止所有输出，避免切换时遗留上一个模式的动作 */
    if (mode != last_mode)
    {
      Fan_Stop();
      Buzzer_Stop();
      LED_Off(LED1);
      LED_Off(LED2);
      last_mode = mode;
    }

    OLED_Clear();
    OLED_ShowString(0, 0, "Mode:", OLED_8X16);
    OLED_ShowNum(40, 0, mode, 2, OLED_8X16);
    switch (mode)
    {
    case 1:
      OLED_ShowString(0, 16, "Fan Forward", OLED_8X16);
      Fan_Forward();
      break;
    case 2:
      OLED_ShowString(0, 16, "Fan Reverse", OLED_8X16);
      Fan_Reverse();
      break;
    case 3:
      OLED_ShowString(0, 16, "Fan Fwd-Rev", OLED_8X16);
      OLED_Update(); /* 先上屏，再跑 4.5s 阻塞循环，文字立即可见 */
      Fan_Forward_Reverse_Start();
      break;
    case 4:
      OLED_ShowString(0, 16, "Music", OLED_8X16);
      if (!Music_IsPlaying()) {
        Buzzer_Play(1);  /* 播放《起风了》 */
      }
      break;
    case 5:
      OLED_ShowString(0, 16, "LED Blink", OLED_8X16);
      LED_Toggle(LED1);
      LED_Toggle(LED2);
      LED_Toggle(LED3);
      break;
    case 6:
      OLED_ShowString(0, 16, "Light Auto", OLED_8X16);
      Light_AutoLED();
      break;
    case 7:
      OLED_ShowString(0, 16, "Fan Light", OLED_8X16);
      Fan_LightControl();
      break;
    case 8:
      OLED_ShowString(0, 16, "Light:", OLED_8X16);
      OLED_ShowNum(56, 16, Light_GetValue(), 4, OLED_8X16);
      break;
    case 9:
    {
      static uint8_t last_dark = 0;
      uint8_t is_dark = (Light_GetValue() >= THRESHOLD_DARK) ? 1 : 0;
      if (is_dark != last_dark)
      {
        last_dark = is_dark;
        OLED_Clear();
      }
      if (is_dark)
      {
        OLED_ShowImage(0, 0, 128, 64, Image_Dark);
      }
      else
      {
        OLED_ShowImage(0, 0, 128, 64, Image_Bright);
      }
    }
    break;
    case 10:
      OLED_ShowString(0, 16, "Key Toggle", OLED_8X16);
      Key_led_toggle_init();
      break;
    case 11:
      OLED_ShowString(0, 16, "Servo:", OLED_8X16);
      OLED_ShowNum(56, 16, servo_angle, 3, OLED_8X16);
      /* PA4 角度增加，PA5 角度减少 */
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
      {
        if (servo_angle < 180)
          servo_angle += 5;
        Servo_SetAngle(servo_angle);
        HAL_Delay(100);
      }
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET)
      {
        if (servo_angle > 0)
          servo_angle -= 5;
        Servo_SetAngle(servo_angle);
        HAL_Delay(100);
      }
      break;
    }

    OLED_Update();
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

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA3 PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 — 先输出高再切 AF_PP，防止 UART 前浮空产生虚假起始位 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure peripheral I/O remapping */
  __HAL_AFIO_REMAP_I2C1_ENABLE();

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
