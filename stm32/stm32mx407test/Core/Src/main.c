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
#include "beep.h"
#include "can.h"
// #include "count_sensor.h"
#include "fan.h"
#include "led.h"
#include "oled.h"
#include <stdint.h>

#define CAN_ROLE_SENDER 0

/* 测试开关：不用按键，两块板都每 1 秒自动发一帧 0x123。
   目的是让总线上"一直有帧"，这样接收端的 EDG/N 才有确定的判据，
   排除"采样那一刻恰好在空闲"的偶然。调完改成 0 即可。 */
#define CAN_TEST_AUTO_TX 1

/* 自检画面开关：现在只用灯做指示，设 0 把整页寄存器画面关掉（代码留着） */
#define CAN_DIAG_OLED 0

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
CAN_HandleTypeDef hcan1;

/* USER CODE BEGIN PV */
volatile uint8_t g_can_rx_flag = 0;
volatile uint32_t g_can_rx_id = 0;
volatile uint8_t g_can_rx_len = 0;
volatile uint8_t g_can_rx_data[8] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
/* USER CODE BEGIN PFP */
#if CAN_DIAG_OLED == 1
static void Show_CAN_Diag(void);
static void Show_CAN_Regs_Page(const CAN_Diag_t *d);
#endif

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if CAN_DIAG_OLED == 1
/* 自检用：ID == 0x123 的回调命中次数 */
static uint32_t s_rx_match_cnt = 0;
#endif

/* 点灯测试用的时间戳 / 计数快照 */
static uint32_t s_led_tick = 0;      /* 心跳灯 */
static uint32_t s_tx_tick = 0;       /* 自动发送 */
static uint32_t s_rx_led_until = 0;  /* 收帧后灯2 亮到什么时候 */
static uint32_t s_rx_ok_last = 0;    /* 上次看到的收帧计数 */

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
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */

  OLED_Init();
  // uint32_t last_count = 0xFFFFFFFF;
  // CountSensor_Init();

  BSP_CAN_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* ---- 灯1：500 ms 翻转一次 = 程序在跑（心跳，不阻塞） ---- */
    if ((HAL_GetTick() - s_led_tick) >= 500U) {
      s_led_tick = HAL_GetTick();
      LED_Toggle(0); /* 灯1 */
    }

    /* ---- 灯2：只要 FIFO0 收到帧（不限 ID）就亮 50 ms，视觉上是"每来一帧闪一下" ---- */
    if (g_can_rx_ok_cnt != s_rx_ok_last) {
      s_rx_ok_last = g_can_rx_ok_cnt;
      LED_On(1); /* 灯2 */
      s_rx_led_until = HAL_GetTick() + 50U;
    }
    if ((s_rx_led_until != 0U) &&
        ((HAL_GetTick() - s_rx_led_until) < 0x80000000U)) {
      s_rx_led_until = 0U;
      LED_Off(1);
    }

#if CAN_TEST_AUTO_TX == 1
    /* ---- 发送：不依赖按键，每秒自动发一帧 ---- */
    if ((HAL_GetTick() - s_tx_tick) >= 1000U) {
      static uint8_t seq = 0;
      uint8_t txbuf[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
      txbuf[0] = seq++;
      MyCAN_Transmit(0x123, 8, txbuf);
      s_tx_tick = HAL_GetTick();
    }
#endif

#if CAN_DIAG_OLED == 1
    /* ---- 寄存器自检画面（现在关着，把 CAN_DIAG_OLED 改成 1 就回来） ---- */
    Show_CAN_Diag();
    OLED_Update();
#endif

    /* 【按需求注释掉】原来的按键触发发送 + OLED 收发显示：
     按键用的 PA0 本来也没在 MX_GPIO_Init 里配置，去掉更干净。
     需要时把下面整段取消注释即可。

#if CAN_ROLE_SENDER == 1
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
      LED_On(3);
      HAL_Delay(20);
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
        uint8_t txbuf[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};
        MyCAN_Transmit(0x123, 8, txbuf);
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
          ;
      }
    }
#else
    if (g_can_rx_flag) {
      g_can_rx_flag = 0;
    }
#endif
    */

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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PF8 PF9 PF10 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PE13 PE14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
#if CAN_DIAG_OLED == 1
/*------------------------------------------------------------------
 * 翻页前把 8 行整行擦成空格（一行 21 字符 = 126 px）。
 * 上一页较长的行不清掉的话，下一页较短的行末尾会留残影。
 *----------------------------------------------------------------*/
static void OLED_ClearPage(void) {
  const char *blank = "                     "; /* 21 个空格 */
  for (uint8_t y = 0; y < 64; y += 8) {
    OLED_ShowString(0, y, (char *)blank, OLED_6X8);
  }
}

#if CAN_ROLE_SENDER == 0
/*------------------------------------------------------------------
 * 在 MCU 引脚自己身上看 CAN_RX（PD0）到底有没有动：
 * AF 模式下 GPIOD->IDR 依然反映引脚电平，所以用紧凑循环采样约 20 ms，
 * 数跳变次数（500 k 每一位 2 µs，线上只要有帧在跑就是几百上千次）。
 *   EDG 大 + N 不涨  → 引脚在动但外设没收到（回到软件侧）
 *   EDG 恒 0         → PD0 电气上根本没动（线断 / 收发器没往 MCU 输出）
 *----------------------------------------------------------------*/
static uint16_t Sample_RX_Edges(void) {
  uint32_t last = GPIOD->IDR & GPIO_PIN_0;
  uint16_t edges = 0;
  uint32_t t0 = HAL_GetTick();

  /* 采满 1 秒：发送端那帧一直挂着重传、总线上是连续活动，
     1 秒窗口足够避开"刚好采到忙/空闲间隙"的偶然 */
  while ((HAL_GetTick() - t0) < 1000U) {
    uint32_t now = GPIOD->IDR & GPIO_PIN_0;
    if (now != last) {
      last = now;
      if (edges < 65535U) {
        edges++; /* 饱和，避免回绕成小数字 */
      }
    }
  }
  return edges;
}
#endif

/*------------------------------------------------------------------
 * 自检第 2 页：PD0/PD1 的 GPIO 配置 + CAN1 关键寄存器
 * 期望值
 *   MODER : 0000000A  PD0/PD1 = AF 模式（0b10）
 *   AFR0  : 00000099  PD0/PD1 都复用成 AF9（CAN1）
 *   IDR   : 00000003  静默时 PD0(隐性)、PD1 都应该是高
 *   MSR   : 00000C00  INAK=0、SLAK=0；复位值是 00000C02（SLAK=1 睡着）
 *   IER   : 00000002  FIFO0 消息中断已使能（bit1）
 *   FA1R  : 00000001  bank0 激活
 *   FM1:0（掩码模式） FS1:1（32 位） FFA:0（挂 FIFO0）
 *----------------------------------------------------------------*/
static void Show_CAN_Regs_Page(const CAN_Diag_t *d) {
  OLED_ShowString(0, 0, "PG2 PIN/CAN1 REG", OLED_6X8);

  OLED_ShowString(0, 8, "MODER:", OLED_6X8);
  OLED_ShowHexNum(36, 8, d->gpiod_moder, 8, OLED_6X8);

  OLED_ShowString(0, 16, "AFR0 :", OLED_6X8);
  OLED_ShowHexNum(36, 16, d->gpiod_afr0, 8, OLED_6X8);

  OLED_ShowString(0, 24, "IDR  :", OLED_6X8);
  OLED_ShowHexNum(36, 24, d->gpiod_idr, 8, OLED_6X8);

  OLED_ShowString(0, 32, "MSR  :", OLED_6X8);
  OLED_ShowHexNum(36, 32, d->msr, 8, OLED_6X8);

  OLED_ShowString(0, 40, "IER  :", OLED_6X8);
  OLED_ShowHexNum(36, 40, d->ier, 8, OLED_6X8);

  OLED_ShowString(0, 48, "FA1R :", OLED_6X8);
  OLED_ShowHexNum(36, 48, d->fa1r, 8, OLED_6X8);

  OLED_ShowString(0, 56, "FM1:", OLED_6X8);
  OLED_ShowNum(24, 56, d->fm1r & 0x01U, 1, OLED_6X8);
  OLED_ShowString(30, 56, " FS1:", OLED_6X8);
  OLED_ShowNum(60, 56, d->fs1r & 0x01U, 1, OLED_6X8);
  OLED_ShowString(66, 56, " FFA:", OLED_6X8);
  OLED_ShowNum(96, 56, d->ffa1r & 0x01U, 1, OLED_6X8);
}

/*------------------------------------------------------------------
 * CAN 自检画面：把 CAN1 的寄存器直接画到 OLED（6x8 字体，8 行）
 *   每行 21 字符以内，横坐标按 6 像素/字符排
 *----------------------------------------------------------------*/
static void Show_CAN_Diag(void) {
  CAN_Diag_t d;
  CAN_Diag_Read(&d);

  OLED_ClearPage(); /* 先整屏擦干净，不然翻页会留上一页的残字 */

  /* 两页轮播：每 2 秒翻一页（第 2 页 = PD0/PD1 配置 + CAN1 关键寄存器） */
  static uint8_t page = 0;
  page ^= 1; /* 每刷一屏翻一页（EDG 采样要 1 秒，所以两页各约 2 秒） */
  if (page != 0) {
    Show_CAN_Regs_Page(&d);
    return;
  }

#if CAN_ROLE_SENDER == 1
  /* ---- 发送端：只关心"发出去没有" ---- */
  // TX REQ: 调用发送的次数
  OLED_ShowString(0, 0, "TX REQ:", OLED_6X8);
  OLED_ShowNum(42, 0, d.tx_req_cnt, 4, OLED_6X8);

  // TSR: TXOK0[0]=1 发送成功  TERR0[15]=1 发送失败  TME0[26]=1 邮箱空
  OLED_ShowString(0, 8, "TSR:", OLED_6X8);
  OLED_ShowHexNum(24, 8, d.tsr, 8, OLED_6X8);

  // ESR: LEC[6:4] 错误类型  TEC[23:16] 发送错误计数  REC[31:24] 接收错误计数
  OLED_ShowString(0, 16, "ESR:", OLED_6X8);
  OLED_ShowHexNum(24, 16, d.esr, 8, OLED_6X8);

  OLED_ShowString(0, 24, "LEC:", OLED_6X8);
  OLED_ShowNum(24, 24, (d.esr >> 4) & 0x07U, 1, OLED_6X8);
  OLED_ShowString(30, 24, " TEC:", OLED_6X8);
  OLED_ShowNum(60, 24, (d.esr >> 16) & 0xFFU, 3, OLED_6X8);
  OLED_ShowString(78, 24, " REC:", OLED_6X8);
  OLED_ShowNum(108, 24, (d.esr >> 24) & 0xFFU, 3, OLED_6X8);

  // MSR: INAK[0]=0 且 SLAK[1]=0 才是正常运行（复位值 C02 里 SLAK=1，是睡着）
  OLED_ShowString(0, 32, "MSR:", OLED_6X8);
  OLED_ShowHexNum(24, 32, d.msr, 8, OLED_6X8);

  // ERR: HAL 汇总的错误标志
  OLED_ShowString(0, 40, "ERR:", OLED_6X8);
  OLED_ShowHexNum(24, 40, d.err, 8, OLED_6X8);

  // 顺便看自己能不能收
  OLED_ShowString(0, 48, "RX N:", OLED_6X8);
  OLED_ShowNum(30, 48, d.rx_irq_cnt, 4, OLED_6X8);

  // FMR: FINIT[0] + CAN2SB[13:8]（CAN2SB=0 → CAN1 没有过滤 bank）
  OLED_ShowString(0, 56, "FMR:", OLED_6X8);
  OLED_ShowHexNum(24, 56, d.fmr, 8, OLED_6X8);
#else
  /* ---- 接收端 ---- */
  // N: 进 RX FIFO0 中断的次数（帧真进来了才涨） FMP: FIFO0 里积压帧数
  // OK: ID 命中 0x123 的次数
  OLED_ShowString(0, 0, "N:", OLED_6X8);
  OLED_ShowNum(12, 0, d.rx_irq_cnt, 4, OLED_6X8);
  OLED_ShowString(36, 0, " FMP:", OLED_6X8);
  OLED_ShowNum(66, 0, d.rf0r & 0x03U, 1, OLED_6X8);
  OLED_ShowString(72, 0, " OK:", OLED_6X8);
  OLED_ShowNum(96, 0, s_rx_match_cnt, 4, OLED_6X8);

  // ESR: LEC[6:4] 错误类型  TEC[23:16] 发送错误计数  REC[31:24] 接收错误计数
  OLED_ShowString(0, 8, "ESR:", OLED_6X8);
  OLED_ShowHexNum(24, 8, d.esr, 8, OLED_6X8);

  OLED_ShowString(0, 16, "LEC:", OLED_6X8);
  OLED_ShowNum(24, 16, (d.esr >> 4) & 0x07U, 1, OLED_6X8);
  OLED_ShowString(30, 16, " TEC:", OLED_6X8);
  OLED_ShowNum(60, 16, (d.esr >> 16) & 0xFFU, 3, OLED_6X8);
  OLED_ShowString(78, 16, " REC:", OLED_6X8);
  OLED_ShowNum(108, 16, (d.esr >> 24) & 0xFFU, 3, OLED_6X8);

  // EDG: 在 MCU 引脚上采样 PD0(CAN_RX) 约 20 ms 的跳变次数。
  // 线上有帧在跑（发送端一直重传）= 几百上千；恒 0 = PD0 电气上没动过
  OLED_ShowString(0, 24, "EDG:", OLED_6X8);
  OLED_ShowNum(24, 24, Sample_RX_Edges(), 5, OLED_6X8);

  // FR2: bank0 的掩码寄存器（掩码全 0 就是全接收，FR1 的 ID 已无意义，那一行换成 EDG）
  OLED_ShowString(0, 32, "FR2:", OLED_6X8);
  OLED_ShowHexNum(24, 32, d.fr2, 8, OLED_6X8);

  // 收到的 ID / 长度
  OLED_ShowString(0, 40, "ID:", OLED_6X8);
  OLED_ShowHexNum(18, 40, g_can_rx_id, 3, OLED_6X8);
  OLED_ShowString(36, 40, " L:", OLED_6X8);
  OLED_ShowNum(54, 40, g_can_rx_len, 1, OLED_6X8);

  // 8 字节数据（没收到的那几位用空格擦掉，避免留残影）
  OLED_ShowString(0, 48, "D:", OLED_6X8);
  for (uint8_t i = 0; i < 8; i++) {
    if (i < g_can_rx_len) {
      OLED_ShowHexNum(12 + i * 12, 48, g_can_rx_data[i], 2, OLED_6X8);
    } else {
      OLED_ShowString(12 + i * 12, 48, "  ", OLED_6X8);
    }
  }

  // FMR: FINIT[0] + CAN2SB[13:8]   FA1: bank0 是否激活
  OLED_ShowString(0, 56, "FMR:", OLED_6X8);
  OLED_ShowHexNum(24, 56, d.fmr, 8, OLED_6X8);
  OLED_ShowString(72, 56, " FA1:", OLED_6X8);
  OLED_ShowNum(102, 56, d.fa1r & 0x01U, 1, OLED_6X8);
#endif
}

// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//   CountSensor_EXTI_Callback(GPIO_Pin);
// }

#endif /* CAN_DIAG_OLED */

// CAN总线回调
void MyCAN_OnRx(uint32_t ID, uint8_t Length, uint8_t *Data) {
  if (ID == 0x123) {
#if CAN_DIAG_OLED == 1
    s_rx_match_cnt++;
#endif
    g_can_rx_id = ID;
    g_can_rx_len = Length;
    for (uint8_t i = 0; i < Length && i < 8; i++) {
      g_can_rx_data[i] = Data[i];
    }
    g_can_rx_flag = 1;
  }
}
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
  while (1) {
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
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
