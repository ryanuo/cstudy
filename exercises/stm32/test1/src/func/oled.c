#include "oled.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#define OLED_SCL_PIN  GPIO_PIN_8
#define OLED_SDA_PIN  GPIO_PIN_9
#define OLED_PORT    GPIOB
#define OLED_ADDR    0x78

#define I2C_SCL_HIGH()   HAL_GPIO_WritePin(OLED_PORT, OLED_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_LOW()    HAL_GPIO_WritePin(OLED_PORT, OLED_SCL_PIN, GPIO_PIN_RESET)
#define I2C_SDA_HIGH()   HAL_GPIO_WritePin(OLED_PORT, OLED_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_LOW()    HAL_GPIO_WritePin(OLED_PORT, OLED_SDA_PIN, GPIO_PIN_RESET)

#define WR_CMD  0x00
#define WR_DATA 0x40

static uint8_t framebuffer[128 * 64 / 8];

static void I2C_Delay(void) {
    for (volatile int i = 0; i < 10; i++);
}

static void I2C_Start(void) {
    I2C_SDA_HIGH();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SDA_LOW();
    I2C_Delay();
    I2C_SCL_LOW();
}

static void I2C_Stop(void) {
    I2C_SDA_LOW();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SDA_HIGH();
    I2C_Delay();
}

static void I2C_WriteByte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i)) I2C_SDA_HIGH();
        else                  I2C_SDA_LOW();
        I2C_Delay();
        I2C_SCL_HIGH();
        I2C_Delay();
        I2C_SCL_LOW();
    }
    I2C_SDA_HIGH();
    I2C_Delay();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SCL_LOW();
}

static void SSD1306_Write(uint8_t type, uint8_t data) {
    I2C_Start();
    I2C_WriteByte(OLED_ADDR);
    I2C_WriteByte(type);
    I2C_WriteByte(data);
    I2C_Stop();
}

void OLED_Refresh(void) {
    for (uint8_t page = 0; page < 8; page++) {
        SSD1306_Write(WR_CMD, 0xB0 | page);
        SSD1306_Write(WR_CMD, 0x00);
        SSD1306_Write(WR_CMD, 0x10);
        I2C_Start();
        I2C_WriteByte(OLED_ADDR);
        I2C_WriteByte(WR_DATA);
        for (uint8_t col = 0; col < 128; col++) {
            I2C_WriteByte(framebuffer[page * 128 + col]);
        }
        I2C_Stop();
    }
}

void OLED_Init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = OLED_SCL_PIN | OLED_SDA_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_PORT, &gpio);
    HAL_GPIO_WritePin(OLED_PORT, OLED_SCL_PIN | OLED_SDA_PIN, GPIO_PIN_SET);

    uint8_t cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    for (uint8_t i = 0; i < sizeof(cmds); i++)
        SSD1306_Write(WR_CMD, cmds[i]);
    memset(framebuffer, 0, sizeof(framebuffer));
    OLED_Refresh();
}

// 简易数字字体 0-9 (8x16)
static const uint8_t digits[][16] = {
    {0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00}, // 0
    {0x00,0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00}, // 1
    {0x00,0x70,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00}, // 2
    {0x00,0x30,0x08,0x88,0x88,0x48,0x30,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00}, // 3
    {0x00,0x00,0xC0,0x20,0x10,0xF8,0x00,0x00,0x00,0x07,0x04,0x24,0x24,0x3F,0x24,0x00}, // 4
    {0x00,0xF8,0x08,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x21,0x20,0x20,0x11,0x0E,0x00}, // 5
    {0x00,0xE0,0x10,0x88,0x88,0x18,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x11,0x0E,0x00}, // 6
    {0x00,0x38,0x08,0x08,0xC8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00}, // 7
    {0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00}, // 8
    {0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x00,0x31,0x22,0x22,0x11,0x0F,0x00}, // 9
};

static void OLED_DrawDigit(uint8_t x, uint8_t y, uint8_t digit) {
    if (digit > 9) return;
    for (uint8_t col = 0; col < 8; col++) {
        framebuffer[y * 128 + x + col] = digits[digit][col];
        framebuffer[(y + 1) * 128 + x + col] = digits[digit][col + 8];
    }
}

void OLED_ShowMode(int mode) {
    memset(framebuffer, 0, sizeof(framebuffer));
    OLED_DrawDigit(60, 3, (uint8_t)mode);  // 居中显示模式号
    OLED_Refresh();
}
