#include <stdint.h>
#include "ssd1306.h"
#include <string.h>

// Software I2C on PB8 (SCL) + PB9 (SDA)
#define I2C_SCL_HIGH()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)
#define I2C_SCL_LOW()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)
#define I2C_SDA_HIGH()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)
#define I2C_SDA_LOW()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)

#define I2C_WR_CMD  0x00
#define I2C_WR_DATA 0x40

static uint8_t framebuffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

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
        if (byte & (1 << i))
            I2C_SDA_HIGH();
        else
            I2C_SDA_LOW();
        I2C_Delay();
        I2C_SCL_HIGH();
        I2C_Delay();
        I2C_SCL_LOW();
    }
    // ACK
    I2C_SDA_HIGH();
    I2C_Delay();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SCL_LOW();
}

static void SSD1306_Write(uint8_t type, uint8_t data) {
    I2C_Start();
    I2C_WriteByte(SSD1306_ADDR);
    I2C_WriteByte(type);
    I2C_WriteByte(data);
    I2C_Stop();
}

void SSD1306_Refresh(void) {
    for (uint8_t page = 0; page < 8; page++) {
        SSD1306_Write(I2C_WR_CMD, 0xB0 | page);
        SSD1306_Write(I2C_WR_CMD, 0x00);
        SSD1306_Write(I2C_WR_CMD, 0x10);
        I2C_Start();
        I2C_WriteByte(SSD1306_ADDR);
        I2C_WriteByte(I2C_WR_DATA);
        for (uint8_t col = 0; col < SSD1306_WIDTH; col++) {
            I2C_WriteByte(framebuffer[page * SSD1306_WIDTH + col]);
        }
        I2C_Stop();
    }
}

void SSD1306_Init(void) {
    // SSD1306 init sequence
    uint8_t cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    for (uint8_t i = 0; i < sizeof(cmds); i++)
        SSD1306_Write(I2C_WR_CMD, cmds[i]);
    SSD1306_ClearScreen();
    SSD1306_Refresh();
}

void SSD1306_ClearScreen(void) {
    memset(framebuffer, 0, sizeof(framebuffer));
}

void SSD1306_FillScreen(void) {
    memset(framebuffer, 0xFF, sizeof(framebuffer));
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    uint16_t idx = (y / 8) * SSD1306_WIDTH + x;
    if (color) framebuffer[idx] |= (1 << (y % 8));
    else       framebuffer[idx] &= ~(1 << (y % 0));
}

// Minimal 5x7 font
static const uint8_t font[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // space
    {0x00,0x5F,0x00,0x00,0x00}, // !
    // ... (simplified - just basic chars)
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
};

void SSD1306_DrawChar(uint8_t x, uint8_t y, char c) {
    if (c < ' ' || c > '9') return;
    uint8_t idx = (c == ' ') ? 0 : (c - '0' + 2);
    if (idx >= sizeof(font)/5) return;
    for (uint8_t i = 0; i < 5; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            if (font[idx][i] & (1 << j))
                SSD1306_DrawPixel(x + i, y + j, 1);
        }
    }
}

void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str) {
    while (*str) {
        SSD1306_DrawChar(x, y, *str++);
        x += 6;
    }
}
