/**
 ******************************************************************************
 * @file    dht11.c
 * @brief   DHT11 温湿度传感器驱动
 *
 *           MCU:
 *               STM32F4
 *
 *           Clock:
 *               HSI = 16MHz
 *
 *           DATA:
 *               PG9
 *
 *           GPIO:
 *               开漏输出 + 外部上拉
 ******************************************************************************
 */
#include "stm32f4xx_hal.h"
#include "dht11.h"

/* ============================================================================
 * DHT11 硬件配置
 * ========================================================================== */

#define DHT11_PORT GPIOG
#define DHT11_PIN GPIO_PIN_9

/* ============================================================================
 * DHT11 时序参数
 * ========================================================================== */

/* MCU 启动 DHT11 时，至少拉低 18ms */
#define DHT11_START_LOW_MS 20U

/* MCU 释放 DATA 后等待 DHT11 响应 */
#define DHT11_START_RELEASE_US 30U

/* DHT11 响应低电平约 80us */
#define DHT11_RESPONSE_LOW_TIMEOUT_US 120U

/* DHT11 响应高电平约 80us */
#define DHT11_RESPONSE_HIGH_TIMEOUT_US 120U

/* 每一位开始时的低电平约 50us */
#define DHT11_BIT_LOW_TIMEOUT_US 100U

/* 每一位高电平最长约 70us */
#define DHT11_BIT_HIGH_TIMEOUT_US 100U

/*
 * DHT11:
 *
 * 0 -> 高电平约 26~28us
 * 1 -> 高电平约 70us
 *
 * 取中间值约 40us 作为判断阈值。
 */
#define DHT11_BIT_THRESHOLD_US 40U

/* ============================================================================
 * DWT 微秒计时
 * ========================================================================== */

/**
 * @brief 初始化 DWT CYCCNT
 *
 * DWT->CYCCNT 用于进行微秒级精确计时。
 */
static void DHT11_DWT_Init(void)
{
    /* 开启 DWT */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* 清零计数器 */
    DWT->CYCCNT = 0;

    /* 开启 CYCCNT */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief 获取当前 DWT 计数值
 */
static uint32_t DHT11_DWT_Get(void)
{
    return DWT->CYCCNT;
}

/**
 * @brief 微秒延时
 *
 * @param us 延时时间，单位：us
 */
static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start;
    uint32_t cycles_per_us;

    start = DHT11_DWT_Get();

    /*
     * 根据当前 HCLK 自动计算：
     *
     * 16MHz -> 16 cycles/us
     * 84MHz -> 84 cycles/us
     * 168MHz -> 168 cycles/us
     */
    cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000U;

    while ((DHT11_DWT_Get() - start) < (us * cycles_per_us))
    {
        /* wait */
    }
}

/**
 * @brief 等待 GPIO 进入指定状态
 *
 * @param state       等待的 GPIO 状态
 * @param timeout_us  超时时间
 *
 * @return 1 = 状态发生变化
 *         0 = 超时
 */
static uint8_t DHT11_WaitForState(GPIO_PinState state,
                                  uint32_t timeout_us)
{
    uint32_t start;
    uint32_t timeout_cycles;

    start = DHT11_DWT_Get();

    timeout_cycles =
        (HAL_RCC_GetHCLKFreq() / 1000000U) * timeout_us;

    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == state)
    {
        if ((DHT11_DWT_Get() - start) >= timeout_cycles)
        {
            return 0;
        }
    }

    return 1;
}

/* ============================================================================
 * GPIO 模式切换
 * ========================================================================== */

/**
 * @brief PG9 设置为开漏输出
 *
 * 开漏输出：
 *
 * LOW  -> MCU 主动拉低
 * HIGH -> MCU 释放总线
 *
 * DHT11 需要外部上拉电阻。
 */
static void DHT11_PinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
 * @brief PG9 设置为输入
 *
 * 此时 MCU 不再驱动 DATA，
 * 由 DHT11 和外部上拉电阻共同控制总线。
 */
static void DHT11_PinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/* ============================================================================
 * 初始化
 * ========================================================================== */

void DHT11_Init(void)
{
    /* 开启 GPIOG 时钟 */
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /* 初始化 DWT */
    DHT11_DWT_Init();

    /* DATA 设置为开漏输出 */
    DHT11_PinOutput();

    /*
     * 空闲状态：
     *
     * MCU 释放 DATA
     * 外部上拉电阻将 DATA 拉高
     */
    HAL_GPIO_WritePin(
        DHT11_PORT,
        DHT11_PIN,
        GPIO_PIN_SET);
}

/* ============================================================================
 * 读取 DHT11
 * ========================================================================== */

/**
 * @brief 读取 DHT11
 *
 * DHT11 返回：
 *
 * data[0] = 湿度整数
 * data[1] = 湿度小数
 * data[2] = 温度整数
 * data[3] = 温度小数
 * data[4] = 校验和
 *
 * @return 1 = 成功
 *         0 = 失败
 */
uint8_t DHT11_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};

    uint8_t i;
    uint8_t j;

    uint32_t start;
    uint32_t high_time_us;

    uint32_t cycles_per_us;

    /* 参数检查 */
    if ((temperature == NULL) || (humidity == NULL))
    {
        return 0;
    }

    /*
     * 确保 DWT 已经启动。
     *
     * 正常情况下 DHT11_Init() 已经初始化。
     */
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0)
    {
        DHT11_DWT_Init();
    }

    cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000U;

    /* ========================================================================
     * 1. 主机发送启动信号
     * ====================================================================== */

    /* DATA 设置为开漏输出 */
    DHT11_PinOutput();

    /* 主机主动拉低 */
    HAL_GPIO_WritePin(
        DHT11_PORT,
        DHT11_PIN,
        GPIO_PIN_RESET);

    /*
     * DHT11 要求：
     *
     * LOW >= 18ms
     */
    HAL_Delay(DHT11_START_LOW_MS);

    /*
     * 释放总线。
     *
     * 开漏输出写 HIGH 并不会主动输出高电平，
     * 而是释放总线。
     */
    HAL_GPIO_WritePin(
        DHT11_PORT,
        DHT11_PIN,
        GPIO_PIN_SET);

    /*
     * 等待约 30us 后切换为输入。
     */
    DHT11_DelayUs(DHT11_START_RELEASE_US);

    /* ========================================================================
     * 2. 等待 DHT11 响应
     * ====================================================================== */

    /* MCU 不再驱动 DATA */
    DHT11_PinInput();

    /*
     * DHT11 会：
     *
     * LOW 约 80us
     * HIGH 约 80us
     */

    /* 等待 DHT11 拉低 */
    if (!DHT11_WaitForState(
            GPIO_PIN_SET,
            DHT11_RESPONSE_LOW_TIMEOUT_US))
    {
        return 0;
    }

    /* 等待低电平结束 */
    if (!DHT11_WaitForState(
            GPIO_PIN_RESET,
            DHT11_RESPONSE_LOW_TIMEOUT_US))
    {
        return 0;
    }

    /* 等待高电平结束 */
    if (!DHT11_WaitForState(
            GPIO_PIN_SET,
            DHT11_RESPONSE_HIGH_TIMEOUT_US))
    {
        return 0;
    }

    /* ========================================================================
     * 3. 读取 40bit 数据
     * ====================================================================== */

    for (j = 0; j < 5; j++)
    {
        for (i = 0; i < 8; i++)
        {
            /*
             * 每一位开始：
             *
             * LOW 约 50us
             *
             * 所以首先等待 LOW 结束。
             */
            if (!DHT11_WaitForState(
                    GPIO_PIN_RESET,
                    DHT11_BIT_LOW_TIMEOUT_US))
            {
                return 0;
            }

            /*
             * LOW 结束以后进入 HIGH。
             *
             * 从 HIGH 开始计时。
             */
            start = DHT11_DWT_Get();

            /*
             * 等待 HIGH 结束。
             */
            if (!DHT11_WaitForState(
                    GPIO_PIN_SET,
                    DHT11_BIT_HIGH_TIMEOUT_US))
            {
                return 0;
            }

            /*
             * 计算 HIGH 持续时间。
             *
             * DWT 是 CPU cycle：
             *
             * high_time_cycles / cycles_per_us
             *
             * 得到微秒。
             */
            high_time_us =
                (DHT11_DWT_Get() - start) / cycles_per_us;

            /*
             * 左移，为下一 bit 腾位置。
             */
            data[j] <<= 1;

            /*
             * 判断：
             *
             * 约 26~28us -> 0
             * 约 70us    -> 1
             *
             * 40us 作为中间阈值。
             */
            if (high_time_us > DHT11_BIT_THRESHOLD_US)
            {
                data[j] |= 1;
            }
        }
    }

    /* ========================================================================
     * 4. 校验
     * ====================================================================== */

    if ((uint8_t)(data[0] +
                  data[1] +
                  data[2] +
                  data[3]) != data[4])
    {
        return 0;
    }

    /* ========================================================================
     * 5. 转换温湿度
     * ====================================================================== */

    *humidity =
        (float)data[0] +
        ((float)data[1] * 0.1f);

    *temperature =
        (float)data[2] +
        ((float)data[3] * 0.1f);

    return 1;
}
