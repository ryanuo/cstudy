#include "DHT11.h"
#include "esp8266.h"          /* 只用 ESP8266_GetTick() 做 2 秒限流（和 FAN.c/BEEP.c 一样的用法）*/

/* ============ 硬件与时序参数（沿用 test407 那份 DHT11 驱动的取值）============ */
#define DHT11_PORT      GPIOG
#define DHT11_PIN       GPIO_Pin_9

#define DHT11_LOW_MS    20U     /* 启动信号：主机拉低 >= 18ms */
#define DHT11_REL_US    30U     /* 释放总线后等 30us 再切输入 */
#define DHT11_RSP_US    120U    /* 模块响应：低 80us + 高 80us */
#define DHT11_BIT_US    100U    /* 每 bit 先低 50us，再高 26~28us(0) / 70us(1) */
#define DHT11_THRESH_US 40U     /* 用 40us 当 0/1 的分界 */

static uint8_t  dht_temp = 0, dht_humi = 0, dht_ok = 0;
static uint32_t dht_last = 0;

/* ============ DWT 微秒计时（不碰 SysTick，避免和 1ms 滴答/其他模块的延时打架）============ */
static void DHT11_DwtInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t DHT11_Now(void)      { return DWT->CYCCNT; }
static uint32_t DHT11_CycPerUs(void) { return SystemCoreClock / 1000000U; }

static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start = DHT11_Now();
    uint32_t cyc   = us * DHT11_CycPerUs();

    while ((uint32_t)(DHT11_Now() - start) < cyc) { }
}

static void DHT11_DelayMs(uint32_t ms)
{
    while (ms--) DHT11_DelayUs(1000U);
}

/* ============ 引脚方向切换 ============ */
/* 开漏输出：写 0 主动拉低、写 1 释放（靠外部 10K 上拉回高）*/
static void DHT11_PinOut(void)
{
    GPIO_InitTypeDef g;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    g.GPIO_Pin   = DHT11_PIN;
    g.GPIO_Mode  = GPIO_Mode_OUT;
    g.GPIO_OType = GPIO_OType_OD;
    g.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &g);
}

static void DHT11_PinIn(void)
{
    GPIO_InitTypeDef g;

    g.GPIO_Pin   = DHT11_PIN;
    g.GPIO_Mode  = GPIO_Mode_IN;
    g.GPIO_PuPd  = GPIO_PuPd_UP;     /* 板上已有 10K，内部再上拉一道更稳 */
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &g);
}

static uint8_t DHT11_Level(void)
{
    return (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) != Bit_RESET) ? 1 : 0;
}

/* 等到电平离开 level（等它变化）；超时返回 0 */
static uint8_t DHT11_WaitLeave(uint8_t level, uint32_t timeout_us)
{
    uint32_t start = DHT11_Now();
    uint32_t limit = timeout_us * DHT11_CycPerUs();

    while (DHT11_Level() == level)
    {
        if ((uint32_t)(DHT11_Now() - start) >= limit) return 0;
    }
    return 1;
}

/* ============ 对外接口 ============ */

void DHT11_Init(void)
{
    DHT11_DwtInit();
    DHT11_PinOut();
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);      /* 空闲：释放总线 */
}

uint8_t DHT11_Read(uint8_t *temp, uint8_t *humi)
{
    uint8_t  d[5] = {0, 0, 0, 0, 0};
    uint8_t  i, j;
    uint32_t t0;
    uint32_t cyc;

    if ((temp == 0) || (humi == 0)) return 0;

    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0) DHT11_DwtInit();
    cyc = DHT11_CycPerUs();

    /* 1. 主机启动信号：拉低 >=18ms 再释放 */
    DHT11_PinOut();
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);
    DHT11_DelayMs(DHT11_LOW_MS);
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);
    DHT11_DelayUs(DHT11_REL_US);

    /* 2. 等模块响应：先拉低 80us，再拉高 80us */
    DHT11_PinIn();
    if (!DHT11_WaitLeave(1, DHT11_RSP_US)) return 0;   /* 等它拉低 */
    if (!DHT11_WaitLeave(0, DHT11_RSP_US)) return 0;   /* 等低电平结束 */
    if (!DHT11_WaitLeave(1, DHT11_RSP_US)) return 0;   /* 等高电平结束 */

    /* 3. 40 bit 数据：每 bit 先低 50us，再高（0 = 26~28us，1 = 70us）*/
    for (j = 0; j < 5; j++)
    {
        for (i = 0; i < 8; i++)
        {
            if (!DHT11_WaitLeave(0, DHT11_BIT_US)) return 0;   /* 等低电平结束 */

            t0 = DHT11_Now();                                  /* 从高电平开始计时 */
            if (!DHT11_WaitLeave(1, DHT11_BIT_US)) return 0;   /* 等高电平结束 */

            d[j] <<= 1;
            if (((DHT11_Now() - t0) / cyc) > DHT11_THRESH_US) d[j] |= 1;
        }
    }

    /* 4. 校验：前 4 字节之和 == 第 5 字节 */
    if ((uint8_t)(d[0] + d[1] + d[2] + d[3]) != d[4]) return 0;

    *humi = d[0];       /* DHT11 的小数位恒为 0，取整数字节就够 */
    *temp = d[2];
    return 1;
}

void DHT11_Task(void)
{
    uint8_t t = 0, h = 0;

    if ((uint32_t)(ESP8266_GetTick() - dht_last) < 2000U) return;   /* DHT11 两次读取至少隔 1 秒 */
    dht_last = ESP8266_GetTick();

    if (DHT11_Read(&t, &h))
    {
        dht_temp = t;
        dht_humi = h;
        dht_ok   = 1;
    }
    else
    {
        dht_ok = 0;      /* 读失败就保留上一次的值，只把 ok 置 0 */
    }
}

uint8_t DHT11_GetTemp(void) { return dht_temp; }
uint8_t DHT11_GetHumi(void) { return dht_humi; }
uint8_t DHT11_Ok(void)      { return dht_ok; }
