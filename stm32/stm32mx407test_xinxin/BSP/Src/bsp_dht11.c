#include "bsp_dht11.h"
#include "board_pins.h"

/* ================= 引脚配置（真值在 board_pins.h） ================= */
#define DHT11_PORT BOARD_DHT11_PORT
#define DHT11_PIN BOARD_DHT11_PIN

/* ================= 时序参数 ================= */
#define DHT11_LOW_MS 20U    /* 起始信号低电平 >=18ms */
#define DHT11_REL_US 30U    /* 释放后等 30us */
#define DHT11_RSP_US 120U   /* 响应信号超时 80+80us */
#define DHT11_BIT_US 100U   /* 每 bit 超时 */
#define DHT11_THRESH_US 40U /* >40us 判为 1 */

/* ================= DWT 微秒延时 ================= */
static uint32_t s_cpu_mhz = 168;

static void DHT11_DelayInit(void) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  s_cpu_mhz = HAL_RCC_GetHCLKFreq() / 1000000U;
  if (s_cpu_mhz == 0)
    s_cpu_mhz = 168;
}

static void DELAY_us(uint32_t us) {
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * s_cpu_mhz;
  while ((DWT->CYCCNT - start) < ticks)
    ;
}

static void DELAY_ms(uint32_t ms) { HAL_Delay(ms); }

/* ================= 内部状态 ================= */
static uint8_t dht_temp = 0, dht_humi = 0, dht_ok = 0;
static uint8_t dht_tdec = 0, dht_hdec = 0;
static uint32_t dht_last = 0;

/* ================= 引脚方向切换 ================= */
static void DHT11_PinOut(void) {
  GPIO_InitTypeDef g = {0};

  g.Pin = DHT11_PIN;
  g.Mode = GPIO_MODE_OUTPUT_OD; /* 开漏，靠外部上拉拉高 */
  g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_PORT, &g);
}

static void DHT11_PinIn(void) {
  GPIO_InitTypeDef g = {0};

  g.Pin = DHT11_PIN;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_PORT, &g);
}

static uint8_t DHT11_Level(void) {
  return (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

/* 等引脚离开 level 状态，超时返回 0 */
static uint8_t DHT11_WaitLeave(uint8_t level, uint32_t timeout_us) {
  uint32_t waited = 0;

  while (DHT11_Level() == level) {
    DELAY_us(1);
    waited++;
    if (waited >= timeout_us)
      return 0;
  }
  return 1;
}

/* ================= 读一次数据 ================= */
uint8_t DHT11_Read(uint8_t *temp, uint8_t *humi) {
  static uint8_t inited = 0;
  if (!inited) {
    DHT11_DelayInit();
    inited = 1;
  }

  uint8_t d[5] = {0};
  uint8_t i, j;
  uint32_t high_us;

  if ((temp == 0) || (humi == 0))
    return 0;

  /* 1. 起始信号：拉低 >=18ms，再释放 */
  DHT11_PinOut();
  HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
  DELAY_ms(DHT11_LOW_MS);
  HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
  DELAY_us(DHT11_REL_US);

  /* 2. 切输入，等响应：80us 低 → 80us 高 → 80us 低 */
  DHT11_PinIn();
  if (!DHT11_WaitLeave(1, DHT11_RSP_US))
    return 0;
  if (!DHT11_WaitLeave(0, DHT11_RSP_US))
    return 0;
  if (!DHT11_WaitLeave(1, DHT11_RSP_US))
    return 0;

  /* 3. 读 40 bit：每 bit 先 50us 低，再高电平（26~28us=0，70us=1） */
  for (j = 0; j < 5; j++) {
    for (i = 0; i < 8; i++) {
      if (!DHT11_WaitLeave(0, DHT11_BIT_US))
        return 0;

      high_us = 0;
      while (DHT11_Level() == 1) {
        DELAY_us(1);
        high_us++;
        if (high_us > DHT11_BIT_US)
          return 0;
      }

      d[j] <<= 1;
      if (high_us > DHT11_THRESH_US)
        d[j] |= 1;
    }
  }

  /* 4. 校验：前 4 字节之和 == 第 5 字节 */
  if ((uint8_t)(d[0] + d[1] + d[2] + d[3]) != d[4])
    return 0;

  *humi = d[0];    /* 湿度整数 */
  *temp = d[2];    /* 温度整数 */
  dht_hdec = d[1]; /* 湿度小数 */
  dht_tdec = d[3]; /* 温度小数 */
  return 1;
}

/* ================= 周期任务（每 2 秒读一次） ================= */
void DHT11_Task(void) {
  uint8_t t = 0, h = 0;

  if ((uint32_t)(HAL_GetTick() - dht_last) < 2000U)
    return;
  dht_last = HAL_GetTick();

  if (DHT11_Read(&t, &h)) {
    dht_temp = t;
    dht_humi = h;
    dht_ok = 1;
  } else {
    dht_ok = 0;
  }
}

/* ================= 状态查询 ================= */
uint8_t DHT11_GetTemp(void) { return dht_temp; }
uint8_t DHT11_GetTempDec(void) { return dht_tdec; }
uint8_t DHT11_GetHumi(void) { return dht_humi; }
uint8_t DHT11_GetHumiDec(void) { return dht_hdec; }
uint8_t DHT11_Ok(void) { return dht_ok; }

/* ================= 一次性获取温湿度结构体 ================= */
DHT11_Data_t DHT11_GetData(void) {
  DHT11_Data_t data;

  data.temp_int = dht_temp;
  data.temp_dec = dht_tdec;
  data.humi_int = dht_humi;
  data.humi_dec = dht_hdec;
  data.valid = dht_ok;

  return data;
}
