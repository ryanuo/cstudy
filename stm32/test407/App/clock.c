/**
 * @file    clock.c
 * @brief   HSE PLL / HSI PLL 运行时切换
 * @details
 *   两种模式 SYSCLK 都是 168MHz，但参考源精度差两个数量级：
 *     HSE PLL : 8MHz 晶振    /PLLM=4  -> 2MHz -> xPLLN=168 -> 336MHz -> /2 = 168MHz
 *     HSI PLL : 16MHz 内部RC /PLLM=8  -> 2MHz -> xPLLN=168 -> 336MHz -> /2 = 168MHz
 *
 *   切换时必须先把 SYSCLK 从 PLL 切走并关掉 PLL，才能改 PLL 参数，
 *   否则 HAL_RCC_OscConfig() 会返回错误。
 *
 *   切换后 HAL_RCC_ClockConfig() 会按新时钟重算 SysTick 重装载值，
 *   所以调用方必须遵循「先切时钟，再延时」的顺序。
 *
 *   指示：LED1(PF10) 灭 = HSE 模式，亮 = HSI 模式。
 */

#include "stm32f4xx_hal.h"
#include "main.h"
#include "led.h"
#include "clock.h"

static uint8_t s_mode = CLK_MODE_HSE_PLL;

/**
 * @brief  PA8 = MCO1，输出 PLLCLK/5 = 33.6MHz 给示波器观测
 * @note   F4 的 MCO1 只能选 HSI/LSE/HSE/PLLCLK，分频最大 /5
 */
static void MCO1_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin       = GPIO_PIN_8;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init(GPIOA, &gpio);

    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_5);
}

/**
 * @brief  把 SYSCLK 切回 HSI 并关闭 PLL（改 PLL 参数前的必经步骤）
 */
static void Clock_LeavePLL(void)
{
    __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSI);
    while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_HSI)
    {
    }

    __HAL_RCC_PLL_DISABLE();
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) != RESET)
    {
    }
}

/**
 * @brief  公共总线分频：AHB=168MHz, APB1=42MHz, APB2=84MHz
 */
static void Clock_ApplyBusConfig(void)
{
    RCC_ClkInitTypeDef clk = {0};

    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV4;
    clk.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }

    SystemCoreClockUpdate();
}

/**
 * @brief  切换到 HSE PLL 168MHz
 */
static void Clock_Config_HSE(void)
{
    RCC_OscInitTypeDef osc = {0};

    Clock_LeavePLL();

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM       = 4;
    osc.PLL.PLLN       = 168;
    osc.PLL.PLLP       = RCC_PLLP_DIV2;
    osc.PLL.PLLQ       = 4;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    Clock_ApplyBusConfig();
    s_mode = CLK_MODE_HSE_PLL;
}

/**
 * @brief  切换到 HSI PLL 168MHz
 */
static void Clock_Config_HSI(void)
{
    RCC_OscInitTypeDef osc = {0};

    Clock_LeavePLL();

    osc.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState            = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState        = RCC_PLL_ON;
    osc.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM            = 8;
    osc.PLL.PLLN            = 168;
    osc.PLL.PLLP            = RCC_PLLP_DIV2;
    osc.PLL.PLLQ            = 4;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    Clock_ApplyBusConfig();
    s_mode = CLK_MODE_HSI_PLL;
}

void Clock_Init(void)
{
    MCO1_Init();
    Clock_Config_HSE();
    LED_Off(LED1);          /* LED1 灭 = HSE 模式 */
}

void Clock_Toggle(void)
{
    if (s_mode == CLK_MODE_HSE_PLL)
    {
        Clock_Config_HSI();
        LED_On(LED1);       /* LED1 亮 = HSI 模式 */
    }
    else
    {
        Clock_Config_HSE();
        LED_Off(LED1);
    }
}

uint8_t Clock_GetMode(void)
{
    return s_mode;
}

/**
 * @brief  读出 MCO1 当前实际配置的输出频率
 * @note   分频和时钟源都从 RCC->CFGR 读回来，改 MCO1_Init() 的配置后
 *         屏幕会自动跟着变。这是按寄存器算的标称值，实测偏差需示波器。
 */
uint32_t Clock_GetMco1Freq(void)
{
    uint32_t div = 1U;

    switch (RCC->CFGR & RCC_CFGR_MCO1PRE)
    {
        case RCC_MCODIV_2: div = 2U; break;
        case RCC_MCODIV_3: div = 3U; break;
        case RCC_MCODIV_4: div = 4U; break;
        case RCC_MCODIV_5: div = 5U; break;
        default:           div = 1U; break;     /* RCC_MCODIV_1 */
    }

    switch (RCC->CFGR & RCC_CFGR_MCO1)
    {
        case RCC_MCO1SOURCE_LSE:    return 32768U / div;
        case RCC_MCO1SOURCE_HSE:    return HSE_VALUE / div;
        case RCC_MCO1SOURCE_PLLCLK: return HAL_RCC_GetSysClockFreq() / div;
        default:                    return HSI_VALUE / div;   /* RCC_MCO1SOURCE_HSI */
    }
}
