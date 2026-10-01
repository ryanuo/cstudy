#include "DHT11.h"
#include "DELAY.h"
#include "sys.h"
#include "esp8266.h"          /* 只用 ESP8266_GetTick() 做 2 秒限流 */

/* ============ 硬件与时序参数 ============ */
#define DHT11_PORT      GPIOG
#define DHT11_PIN       GPIO_Pin_9

#define DHT11_LOW_MS    20U     /* 启动信号：主机拉低 >= 18ms */
#define DHT11_REL_US    30U     /* 释放总线后等 30us 再切输入 */
#define DHT11_RSP_US    120U    /* 模块响应：低 80us + 高 80us */
#define DHT11_BIT_US    100U    /* 每 bit 先低 50us，再高 26~28us(0) / 70us(1) */
#define DHT11_THRESH_US 40U     /* 用 40us 当 0/1 的分界 */

static uint8_t  dht_temp = 0, dht_humi = 0, dht_ok = 0;
static uint8_t  dht_tdec = 0, dht_hdec = 0;
static uint32_t dht_last = 0;

/* ============ 引脚方向切换 ============ */
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
    g.GPIO_PuPd  = GPIO_PuPd_UP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &g);
}

static uint8_t DHT11_Level(void)
{
    return (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) != Bit_RESET) ? 1 : 0;
}

/* 等电平离开 level；用 DELAY_us 步进计时，超时返回 0。
   每次等 1us，最多等 timeout_us 次。 */
static uint8_t DHT11_WaitLeave(uint8_t level, uint32_t timeout_us)
{
    uint32_t waited = 0;

    while (DHT11_Level() == level)
    {
        DELAY_us(1);
        waited++;
        if (waited >= timeout_us) return 0;
    }
    return 1;
}

/* ============ 对外接口 ============ */

void DHT11_Init(void)
{
    DHT11_PinOut();
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);      /* 空闲：释放总线 */
}

uint8_t DHT11_Read(uint8_t *temp, uint8_t *humi)
{
    uint8_t  d[5] = {0, 0, 0, 0, 0};
    uint8_t  i, j;
    uint32_t high_us;

    if ((temp == 0) || (humi == 0)) return 0;

    /* 1. 主机启动信号：拉低 >=18ms 再释放 */
    DHT11_PinOut();
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);
    DELAY_ms(DHT11_LOW_MS);
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);
    DELAY_us(DHT11_REL_US);

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

            /* 从高电平开始，用 DELAY_us 步进统计高电平持续了多少 us */
            high_us = 0;
            while (DHT11_Level() == 1)
            {
                DELAY_us(1);
                high_us++;
                if (high_us > DHT11_BIT_US) return 0;   /* 超时保护 */
            }

            d[j] <<= 1;
            if (high_us > DHT11_THRESH_US) d[j] |= 1;
        }
    }

    /* 4. 校验：前 4 字节之和 == 第 5 字节 */
    if ((uint8_t)(d[0] + d[1] + d[2] + d[3]) != d[4]) return 0;

    *humi = d[0];       /* d[0]=湿度整数 d[1]=湿度小数 d[2]=温度整数 d[3]=温度小数 */
    *temp = d[2];
    dht_hdec = d[1];
    dht_tdec = d[3];
    return 1;
}

void DHT11_Task(void)
{
    uint8_t t = 0, h = 0;

    if ((uint32_t)(ESP8266_GetTick() - dht_last) < 2000U) return;
    dht_last = ESP8266_GetTick();

    if (DHT11_Read(&t, &h))
    {
        dht_temp = t;
        dht_humi = h;
        dht_ok   = 1;
    }
    else
    {
        dht_ok = 0;
    }
}

uint8_t DHT11_GetTemp(void) { return dht_temp; }
uint8_t DHT11_GetTempDec(void) { return dht_tdec; }
uint8_t DHT11_GetHumiDec(void) { return dht_hdec; }
uint8_t DHT11_GetHumi(void) { return dht_humi; }
uint8_t DHT11_Ok(void)      { return dht_ok; }