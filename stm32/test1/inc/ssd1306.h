#ifndef __SSD1306_H
#define __SSD1306_H

#include "stm32f1xx.h"

#define SSD1306_ADDR   0x78  // 0x3C << 1
#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64

void SSD1306_Init(void);
void SSD1306_FillScreen(void);
void SSD1306_ClearScreen(void);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void SSD1306_DrawChar(uint8_t x, uint8_t y, char c);
void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str);
void SSD1306_Refresh(void);

#endif
