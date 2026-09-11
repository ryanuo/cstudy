/**
 * @file    clock.c
 * @brief   运行时切换系统时钟：HSE PLL 168MHz / HSI 直连 16MHz / HSE 直连 8MHz
 * @details
 *   三种模式的 SYSCLK：
 *     HSE PLL : 8MHz 晶振  /PLLM=4 -> 2MHz -> xPLLN=168 -> 336MHz -> /2 = 168MHz
 *     HSI 直连: 16MHz 内部 RC 直接做 SYSCLK（PLL 关闭）
 *     HSE 直连: 8MHz 晶振    直接做 SYSCLK（PLL 关闭）
 *
 *   切换顺序（必须按这个来）：
 *     1. 先把 SYSCLK 从 PLL 切到 HSI，再关 PLL
 *        —— PLL 还当着 SYSCLK 源的时候去改 PLLM/PLLN 会失锁，
 *           HAL_RCC_OscConfig() 会直接返回错误
 *     2. HAL_RCC_OscConfig() 配振荡器 / PLL
 *     3. HAL_RCC_ClockConfig() 切 SYSCLK + 改总线分频 + 改 FLASH 等待周期
 *        （它内部会调 HAL_InitTick()，按新时钟重算 SysTick 重装载值 ——
 *          所以 HAL_Delay() 在任何模式下都还是准的）
 *     4. SystemCoreClockUpdate() 刷新全局 SystemCoreClock
 *     5. MCO1 换源
 *        —— 直连模式下 PLL 是关的，MCO1 若仍选 PLLCLK，PA8 会变成一条直线
 *
 *   调用方必须遵循「先切时钟，再延时」的顺序（见 Core/Src/main.c 主循环）。
 */

#include "stm32f4xx_hal.h"
#include "main.h"
#include "led.h"
#include "clock.h"

static uint8_t s_mode = CLK_MODE_HSE_PLL;

/**
 * @brief  PA8 = MCO1 引脚初始化（复用推挽，AF0）
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
}

/**
 * @brief  换 MCO1 的时钟源和分频
 * @note   F4 的 MCO1 只能选 HSI/LSE/HSE/PLLCLK（没有 SYSCLK），分频最大 /5
 */
static void MCO1_Set(uint32_t source, uint32_t div)
{
    HAL_RCC_MCOConfig(RCC_MCO1, source, div);
}

/**
 * @brief  把 SYSCLK 切回 HSI 并关闭 PLL（改 PLL 参数前的必经步骤）
 * @note   HSI 上电默认就是开的，所以这里总是有可靠的过渡时钟
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
 * @brief  切 SYSCLK 源 + 总线分频 + FLASH 等待周期，并刷新 SystemCoreClock
 * @param  sysclk_src RCC_SYSCLKSOURCE_xxx
 * @param  apb1_div   RCC_HCLK_DIVx
 * @param  apb2_div   RCC_HCLK_DIVx
 * @param  latency    FLASH_LATENCY_x（168MHz 用 5，<30MHz 用 0）
 */
static void Clock_ApplyBus(uint32_t sysclk_src, uint32_t apb1_div,
                           uint32_t apb2_div, uint32_t latency)
{
    RCC_ClkInitTypeDef clk = {0};

    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = sysclk_src;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = apb1_div;
    clk.APB2CLKDivider = apb2_div;

    if (HAL_RCC_ClockConfig(&clk, latency) != HAL_OK)
    {
        Error_Handler();
    }

    /* HAL 自己也会更新这个变量，这里再确认一次，保证后面按新频率算 */
    SystemCoreClockUpdate();
}

/**
 * @brief  切换到 HSE PLL 168MHz（AHB=168, APB1=42, APB2=84）
 */
static void Clock_Config_HSE_PLL(void)
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

    Clock_ApplyBus(RCC_SYSCLKSOURCE_PLLCLK, RCC_HCLK_DIV4, RCC_HCLK_DIV2,
                   FLASH_LATENCY_5);

    MCO1_Set(RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_5);      /* PA8 = 33.6MHz */
}

/**
 * @brief  切换到 HSI 直连 16MHz（PLL 关闭）
 */
static void Clock_Config_HSI_Direct(void)
{
    RCC_OscInitTypeDef osc = {0};

    Clock_LeavePLL();

    osc.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState            = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState        = RCC_PLL_OFF;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    Clock_ApplyBus(RCC_SYSCLKSOURCE_HSI, RCC_HCLK_DIV1, RCC_HCLK_DIV1,
                   FLASH_LATENCY_0);                    /* <30MHz 用 0 等待周期 */

    MCO1_Set(RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);         /* PA8 = 16MHz */
}

/**
 * @brief  切换到 HSE 直连 8MHz（PLL 关闭）
 */
static void Clock_Config_HSE_Direct(void)
{
    RCC_OscInitTypeDef osc = {0};

    Clock_LeavePLL();

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.PLL.PLLState   = RCC_PLL_OFF;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    Clock_ApplyBus(RCC_SYSCLKSOURCE_HSE, RCC_HCLK_DIV1, RCC_HCLK_DIV1,
                   FLASH_LATENCY_0);

    MCO1_Set(RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);         /* PA8 = 8MHz */
}

void Clock_Init(void)
{
    MCO1_Init();
    Clock_Config_HSE_PLL();
    s_mode = CLK_MODE_HSE_PLL;
}

void Clock_SetMode(uint8_t mode)
{
    switch (mode)
    {
        case CLK_MODE_HSE_PLL:    Clock_Config_HSE_PLL();    break;
        case CLK_MODE_HSI_DIRECT: Clock_Config_HSI_Direct(); break;
        case CLK_MODE_HSE_DIRECT: Clock_Config_HSE_Direct(); break;
        default: return;
    }

    s_mode = mode;
}

void Clock_Toggle(void)
{
    uint8_t next = (uint8_t)(s_mode + 1U);

    if (next >= CLK_MODE_COUNT)
    {
        next = CLK_MODE_HSE_PLL;
    }

    Clock_SetMode(next);
}

uint8_t Clock_GetMode(void)
{
    return s_mode;
}

const char *Clock_GetModeName(void)
{
    switch (s_mode)
    {
        case CLK_MODE_HSE_PLL:    return "HSE PLL";
        case CLK_MODE_HSI_DIRECT: return "HSI 16M";
        default:                  return "HSE  8M";
    }
}

/**
 * @brief  读出 MCO1 当前实际配置的输出频率
 * @note   分频和时钟源都从 RCC->CFGR 读回来，MCO1 换源/换分频后
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
