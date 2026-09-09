
/***************************************************************************************
  * @file    OLED.c
  * @brief   SSD1306 OLED (128x64) driver, software I2C on PB8/PB9
  * @note    Drawing functions operate on OLED_DisplayBuf; call OLED_Update() to refresh.
  ***************************************************************************************
  */

#include "stm32f4xx_hal.h"
#include "OLED.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/* 软件 I2C 引脚定义 */
#define OLED_SCL_PORT  GPIOB
#define OLED_SCL_PIN   GPIO_PIN_8
#define OLED_SDA_PORT  GPIOB
#define OLED_SDA_PIN   GPIO_PIN_9

static GPIO_InitTypeDef oled_gpio = {0};

static void OLED_W_SCL(uint8_t BitValue) {
    HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, (GPIO_PinState)BitValue);
}

static void OLED_W_SDA(uint8_t BitValue) {
    HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, (GPIO_PinState)BitValue);
}

static void OLED_GPIO_Init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    oled_gpio.Pin = OLED_SCL_PIN | OLED_SDA_PIN;
    oled_gpio.Mode = GPIO_MODE_OUTPUT_OD;
    oled_gpio.Pull = GPIO_NOPULL;
    oled_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &oled_gpio);
    HAL_GPIO_WritePin(GPIOB, OLED_SCL_PIN | OLED_SDA_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Bitmap data layout used by this driver
  *
  *      B0 B0                  B0 B0
  *      B1 B1                  B1 B1
  *      B2 B2                  B2 B2
  *      B3 B3  ------------->  B3 B3 --
  *      B4 B4                  B4 B4  |
  *      B5 B5                  B5 B5  |
  *      B6 B6                  B6 B6  |
  *      B7 B7                  B7 B7  |
  *                                    |
  *  -----------------------------------
  *  |   
  *  |   B0 B0                  B0 B0
  *  |   B1 B1                  B1 B1
  *  |   B2 B2                  B2 B2
  *  --> B3 B3  ------------->  B3 B3
  *      B4 B4                  B4 B4
  *      B5 B5                  B5 B5
  *      B6 B6                  B6 B6
  *      B7 B7                  B7 B7
  * 
  * 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *      |
  *      |
  *      |
  *   63 |
  *      v
  * 
  */



/**
  * @brief  OLED display buffer, 8 pages x 128 columns
  */
uint8_t OLED_DisplayBuf[8][128];









/**
  * @brief  Generate I2C start condition
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  Generate I2C stop condition
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  Send one byte over software I2C, MSB first
  * @param  Byte: data byte to send
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(!!(Byte & (0x80 >> i)));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	
	OLED_W_SCL(1);
	OLED_W_SCL(0);
}

/**
  * @brief  Write one command byte to OLED
  * @param  Command: command byte
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);
	OLED_I2C_SendByte(0x00);
	OLED_I2C_SendByte(Command);
	OLED_I2C_Stop();
}

/**
  * @brief  Write a burst of display data to OLED (GDDRAM)
  * @param  Data: data buffer, Count: number of bytes
  */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
	uint8_t i;
	
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);
	OLED_I2C_SendByte(0x40);
	for (i = 0; i < Count; i ++)
	{
		OLED_I2C_SendByte(Data[i]);
	}
	OLED_I2C_Stop();
}




/**
  * @brief  Initialize OLED: GPIO, software I2C and SSD1306 registers
  */
void OLED_Init(void)
{
	OLED_GPIO_Init();
	
	OLED_WriteCommand(0xAE);
	
	OLED_WriteCommand(0xD5);
	OLED_WriteCommand(0x80);	//0x00~0xFF
	
	OLED_WriteCommand(0xA8);
	OLED_WriteCommand(0x3F);	//0x0E~0x3F
	
	OLED_WriteCommand(0xD3);
	OLED_WriteCommand(0x00);	//0x00~0x7F
	
	OLED_WriteCommand(0x40);
	
	OLED_WriteCommand(0xA1);
	
	OLED_WriteCommand(0xC8);

	OLED_WriteCommand(0xDA);
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);
	OLED_WriteCommand(0xCF);	//0x00~0xFF

	OLED_WriteCommand(0xD9);
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);

	OLED_WriteCommand(0xA6);

	OLED_WriteCommand(0x8D);
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);
	
	OLED_Clear();
	OLED_Update();
}

/**
  * @brief  Set cursor position
  * @param  Page: page address 0~7, X: column address 0~127
  */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
//	X += 2;
	
	OLED_WriteCommand(0xB0 | Page);
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
	OLED_WriteCommand(0x00 | (X & 0x0F));
}





/**
  * @brief  Integer power: X raised to the power of Y
  * @param  X: base, Y: exponent
  * @retval X^Y
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  Point-in-polygon test (ray casting algorithm)
  * @param  nvert: vertex count, vertx/verty: vertex arrays, testx/testy: point to test
  * @retval 1 if inside, 0 otherwise
  */
uint8_t OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
	int16_t i, j, c = 0;
	
	for (i = 0, j = nvert - 1; i < nvert; j = i++)
	{
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
		{
			c = !c;
		}
	}
	return c;
}

/**
  * @brief  Check whether a point falls inside the given angle range
  * @param  X/Y: point, StartAngle/EndAngle: angle range in degrees
  * @retval 1 if inside, 0 otherwise
  */
uint8_t OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
	int16_t PointAngle;
	PointAngle = atan2(Y, X) / 3.14 * 180;
	if (StartAngle < EndAngle)
	{
		if (PointAngle >= StartAngle && PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	else
	{
		if (PointAngle >= StartAngle || PointAngle <= EndAngle)
		{
			return 1;
		}
	}
	return 0;
}




/**
  * @brief  Push the whole display buffer to OLED (full refresh)
  */
void OLED_Update(void)
{
	uint8_t j;
	for (j = 0; j < 8; j ++)
	{
		OLED_SetCursor(j, 0);
		OLED_WriteData(OLED_DisplayBuf[j], 128);
	}
}

/**
  * @brief  Push only the given rectangular area to OLED (partial refresh)
  * @param  X/Y: top-left, Width/Height: area size
  */
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t j;
	int16_t Page, Page1;
	
	Page = Y / 8;
	Page1 = (Y + Height - 1) / 8 + 1;
	if (Y < 0)
	{
		Page -= 1;
		Page1 -= 1;
	}
	
	for (j = Page; j < Page1; j ++)
	{
		if (X >= 0 && X <= 127 && j >= 0 && j <= 7)
		{
			OLED_SetCursor(j, X);
			OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
		}
	}
}

/**
  * @brief  Clear the whole display buffer
  */
void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)
	{
		for (i = 0; i < 128; i ++)
		{
			OLED_DisplayBuf[j][i] = 0x00;
		}
	}
}

/**
  * @brief  Clear a rectangular area in the display buffer
  * @param  X/Y: top-left, Width/Height: area size
  */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t i, j;
	
	for (j = Y; j < Y + Height; j ++)
	{
		for (i = X; i < X + Width; i ++)
		{
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)
			{
				OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));
			}
		}
	}
}

/**
  * @brief  Invert every pixel in the display buffer
  */
void OLED_Reverse(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)
	{
		for (i = 0; i < 128; i ++)
		{
			OLED_DisplayBuf[j][i] ^= 0xFF;
		}
	}
}
	
/**
  * @brief  Invert pixels in a rectangular area
  * @param  X/Y: top-left, Width/Height: area size
  */
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	int16_t i, j;
	
	for (j = Y; j < Y + Height; j ++)
	{
		for (i = X; i < X + Width; i ++)
		{
			if (i >= 0 && i <= 127 && j >=0 && j <= 63)
			{
				OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);
			}
		}
	}
}

/**
  * @brief  Display a single ASCII character
  * @param  X/Y: top-left, Char: character, FontSize: OLED_8X16 or OLED_6X8
  */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
	if (FontSize == OLED_8X16)
	{
		OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
	}
	else if(FontSize == OLED_6X8)
	{
		OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
	}
}

/**
  * @brief  Display a string, Chinese characters supported
  * @param  X/Y: top-left, String: null-terminated string, FontSize: font size
  */
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
	uint16_t i = 0;
	char SingleChar[5];
	uint8_t CharLength = 0;
	uint16_t XOffset = 0;
	uint16_t pIndex;
	
	while (String[i] != '\0')
	{
		
#ifdef OLED_CHARSET_UTF8
		if ((String[i] & 0x80) == 0x00)
		{
			CharLength = 1;
			SingleChar[0] = String[i ++];
			SingleChar[1] = '\0';
		}
		else if ((String[i] & 0xE0) == 0xC0)
		{
			CharLength = 2;
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			SingleChar[2] = '\0';
		}
		else if ((String[i] & 0xF0) == 0xE0)
		{
			CharLength = 3;
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[2] = String[i ++];
			SingleChar[3] = '\0';
		}
		else if ((String[i] & 0xF8) == 0xF0)
		{
			CharLength = 4;
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[2] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[3] = String[i ++];
			SingleChar[4] = '\0';
		}
		else
		{
			i ++;
			continue;
		}
#endif
		
#ifdef OLED_CHARSET_GB2312
		if ((String[i] & 0x80) == 0x00)
		{
			CharLength = 1;
			SingleChar[0] = String[i ++];
			SingleChar[1] = '\0';
		}
		else
		{
			CharLength = 2;
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			SingleChar[2] = '\0';
		}
#endif
		
		if (CharLength == 1)
		{
			OLED_ShowChar(X + XOffset, Y, SingleChar[0], FontSize);
			XOffset += FontSize;
		}
		else
		{
			for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex ++)
			{
				if (strcmp(OLED_CF16x16[pIndex].Index, SingleChar) == 0)
				{
					break;
				}
			}
			if (FontSize == OLED_8X16)
			{
				OLED_ShowImage(X + XOffset, Y, 16, 16, OLED_CF16x16[pIndex].Data);
				XOffset += 16;
			}
			else if (FontSize == OLED_6X8)
			{
				OLED_ShowChar(X + XOffset, Y, '?', OLED_6X8);
				XOffset += OLED_6X8;
			}
		}
	}
}

/**
  * @brief  Display an unsigned decimal number
  * @param  X/Y: top-left, Number: value, Length: digit count, FontSize: font size
  */
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; i < Length; i++)
	{
		OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * @brief  Display a signed decimal number with + or - sign
  * @param  X/Y: top-left, Number: value, Length: digit count, FontSize: font size
  */
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	uint32_t Number1;
	
	if (Number >= 0)
	{
		OLED_ShowChar(X, Y, '+', FontSize);
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(X, Y, '-', FontSize);
		Number1 = -Number;
	}
	
	for (i = 0; i < Length; i++)
	{
		OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
	}
}

/**
  * @brief  Display a number in hexadecimal
  * @param  X/Y: top-left, Number: value, Length: digit count, FontSize: font size
  */
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		
		if (SingleNumber < 10)
		{
			OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
		}
		else
		{
			OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
		}
	}
}

/**
  * @brief  Display a number in binary
  * @param  X/Y: top-left, Number: value, Length: digit count, FontSize: font size
  */
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
	uint8_t i;
	for (i = 0; i < Length; i++)
	{
		OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
	}
}

/**
  * @brief  Display a floating point number
  * @param  X/Y: top-left, Number: value, IntLength: integer digits, FraLength: fraction digits, FontSize: font size
  */
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
	uint32_t PowNum, IntNum, FraNum;
	
	if (Number >= 0)
	{
		OLED_ShowChar(X, Y, '+', FontSize);
	}
	else
	{
		OLED_ShowChar(X, Y, '-', FontSize);
		Number = -Number;
	}
	
	IntNum = Number;
	Number -= IntNum;
	PowNum = OLED_Pow(10, FraLength);
	FraNum = round(Number * PowNum);
	IntNum += FraNum / PowNum;
	
	OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);
	
	OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);
	
	OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}

/**
  * @brief  Display a monochrome bitmap
  * @param  X/Y: top-left, Width/Height: bitmap size, Image: bitmap data
  */
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
	uint8_t i = 0, j = 0;
	int16_t Page, Shift;
	
	OLED_ClearArea(X, Y, Width, Height);
	
	for (j = 0; j < (Height - 1) / 8 + 1; j ++)
	{
		for (i = 0; i < Width; i ++)
		{
			if (X + i >= 0 && X + i <= 127)
			{
				Page = Y / 8;
				Shift = Y % 8;
				if (Y < 0)
				{
					Page -= 1;
					Shift += 8;
				}
				
				if (Page + j >= 0 && Page + j <= 7)
				{
					OLED_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
				}
				
				if (Page + j + 1 >= 0 && Page + j + 1 <= 7)
				{					
					OLED_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
				}
			}
		}
	}
}

/**
  * @brief  Formatted output to OLED (printf style)
  * @param  X/Y: top-left, FontSize: font size, format: format string
  */
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
	char String[256];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	OLED_ShowString(X, Y, String, FontSize);
}

/**
  * @brief  Set a single pixel in the display buffer
  * @param  X/Y: pixel coordinate
  */
void OLED_DrawPoint(int16_t X, int16_t Y)
{
	if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)
	{
		OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
	}
}

/**
  * @brief  Read a single pixel from the display buffer
  * @param  X/Y: pixel coordinate
  * @retval 1 if lit, 0 otherwise
  */
uint8_t OLED_GetPoint(int16_t X, int16_t Y)
{
	if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)
	{
		if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
		{
			return 1;
		}
	}
	
	return 0;
}

/**
  * @brief  Draw a line using Bresenham algorithm
  * @param  X0/Y0: start point, X1/Y1: end point
  */
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
	int16_t x, y, dx, dy, d, incrE, incrNE, temp;
	int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
	uint8_t yflag = 0, xyflag = 0;
	
	if (y0 == y1)
	{
		if (x0 > x1) {temp = x0; x0 = x1; x1 = temp;}
		
		for (x = x0; x <= x1; x ++)
		{
			OLED_DrawPoint(x, y0);
		}
	}
	else if (x0 == x1)
	{
		if (y0 > y1) {temp = y0; y0 = y1; y1 = temp;}
		
		for (y = y0; y <= y1; y ++)
		{
			OLED_DrawPoint(x0, y);
		}
	}
	else
	{
		
		if (x0 > x1)
		{
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}
		
		if (y0 > y1)
		{
			y0 = -y0;
			y1 = -y1;
			
			yflag = 1;
		}
		
		if (y1 - y0 > x1 - x0)
		{
			temp = x0; x0 = y0; y0 = temp;
			temp = x1; x1 = y1; y1 = temp;
			
			xyflag = 1;
		}
		
		dx = x1 - x0;
		dy = y1 - y0;
		incrE = 2 * dy;
		incrNE = 2 * (dy - dx);
		d = 2 * dy - dx;
		x = x0;
		y = y0;
		
		if (yflag && xyflag){OLED_DrawPoint(y, -x);}
		else if (yflag)		{OLED_DrawPoint(x, -y);}
		else if (xyflag)	{OLED_DrawPoint(y, x);}
		else				{OLED_DrawPoint(x, y);}
		
		while (x < x1)
		{
			x ++;
			if (d < 0)
			{
				d += incrE;
			}
			else
			{
				y ++;
				d += incrNE;
			}
			
			if (yflag && xyflag){OLED_DrawPoint(y, -x);}
			else if (yflag)		{OLED_DrawPoint(x, -y);}
			else if (xyflag)	{OLED_DrawPoint(y, x);}
			else				{OLED_DrawPoint(x, y);}
		}	
	}
}

/**
  * @brief  Draw a rectangle
  * @param  X/Y: top-left, Width/Height: size, IsFilled: 1 filled, 0 outline
  */
void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
	int16_t i, j;
	if (!IsFilled)
	{
		for (i = X; i < X + Width; i ++)
		{
			OLED_DrawPoint(i, Y);
			OLED_DrawPoint(i, Y + Height - 1);
		}
		for (i = Y; i < Y + Height; i ++)
		{
			OLED_DrawPoint(X, i);
			OLED_DrawPoint(X + Width - 1, i);
		}
	}
	else
	{
		for (i = X; i < X + Width; i ++)
		{
			for (j = Y; j < Y + Height; j ++)
			{
				OLED_DrawPoint(i, j);
			}
		}
	}
}

/**
  * @brief  Draw a triangle
  * @param  X0/Y0, X1/Y1, X2/Y2: three vertices, IsFilled: 1 filled, 0 outline
  */
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled)
{
	int16_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
	int16_t i, j;
	int16_t vx[] = {X0, X1, X2};
	int16_t vy[] = {Y0, Y1, Y2};
	
	if (!IsFilled)
	{
		OLED_DrawLine(X0, Y0, X1, Y1);
		OLED_DrawLine(X0, Y0, X2, Y2);
		OLED_DrawLine(X1, Y1, X2, Y2);
	}
	else
	{
		if (X1 < minx) {minx = X1;}
		if (X2 < minx) {minx = X2;}
		if (Y1 < miny) {miny = Y1;}
		if (Y2 < miny) {miny = Y2;}
		
		if (X1 > maxx) {maxx = X1;}
		if (X2 > maxx) {maxx = X2;}
		if (Y1 > maxy) {maxy = Y1;}
		if (Y2 > maxy) {maxy = Y2;}
		
		for (i = minx; i <= maxx; i ++)
		{
			for (j = miny; j <= maxy; j ++)
			{
				if (OLED_pnpoly(3, vx, vy, i, j)) {OLED_DrawPoint(i, j);}
			}
		}
	}
}

/**
  * @brief  Draw a circle using midpoint algorithm
  * @param  X/Y: center, Radius: radius, IsFilled: 1 filled, 0 outline
  */
void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled)
{
	int16_t x, y, d, j;
	
	
	d = 1 - Radius;
	x = 0;
	y = Radius;
	
	OLED_DrawPoint(X + x, Y + y);
	OLED_DrawPoint(X - x, Y - y);
	OLED_DrawPoint(X + y, Y + x);
	OLED_DrawPoint(X - y, Y - x);
	
	if (IsFilled)
	{
		for (j = -y; j < y; j ++)
		{
			OLED_DrawPoint(X, Y + j);
		}
	}
	
	while (x < y)
	{
		x ++;
		if (d < 0)
		{
			d += 2 * x + 1;
		}
		else
		{
			y --;
			d += 2 * (x - y) + 1;
		}
		
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X + y, Y + x);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - y, Y - x);
		OLED_DrawPoint(X + x, Y - y);
		OLED_DrawPoint(X + y, Y - x);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X - y, Y + x);
		
		if (IsFilled)
		{
			for (j = -y; j < y; j ++)
			{
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}
			
			for (j = -x; j < x; j ++)
			{
				OLED_DrawPoint(X - y, Y + j);
				OLED_DrawPoint(X + y, Y + j);
			}
		}
	}
}

/**
  * @brief  Draw an ellipse using midpoint algorithm
  * @param  X/Y: center, A: semi-axis X, B: semi-axis Y, IsFilled: 1 filled, 0 outline
  */
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
{
	int16_t x, y, j;
	int16_t a = A, b = B;
	float d1, d2;
	
	
	x = 0;
	y = b;
	d1 = b * b + a * a * (-b + 0.5);
	
	if (IsFilled)
	{
		for (j = -y; j < y; j ++)
		{
			OLED_DrawPoint(X, Y + j);
			OLED_DrawPoint(X, Y + j);
		}
	}
	
	OLED_DrawPoint(X + x, Y + y);
	OLED_DrawPoint(X - x, Y - y);
	OLED_DrawPoint(X - x, Y + y);
	OLED_DrawPoint(X + x, Y - y);
	
	while (b * b * (x + 1) < a * a * (y - 0.5))
	{
		if (d1 <= 0)
		{
			d1 += b * b * (2 * x + 3);
		}
		else
		{
			d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
			y --;
		}
		x ++;
		
		if (IsFilled)
		{
			for (j = -y; j < y; j ++)
			{
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}
		}
		
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X + x, Y - y);
	}
	
	d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;
	
	while (y > 0)
	{
		if (d2 <= 0)
		{
			d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
			x ++;
			
		}
		else
		{
			d2 += a * a * (-2 * y + 3);
		}
		y --;
		
		if (IsFilled)
		{
			for (j = -y; j < y; j ++)
			{
				OLED_DrawPoint(X + x, Y + j);
				OLED_DrawPoint(X - x, Y + j);
			}
		}
		
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X + x, Y - y);
	}
}

/**
  * @brief  Draw an arc between two angles
  * @param  X/Y: center, Radius: radius, StartAngle/EndAngle: range in degrees, IsFilled: 1 filled, 0 outline
  */
void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
	int16_t x, y, d, j;
	
	
	d = 1 - Radius;
	x = 0;
	y = Radius;
	
	if (OLED_IsInAngle(x, y, StartAngle, EndAngle))	{OLED_DrawPoint(X + x, Y + y);}
	if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
	if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
	if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
	
	if (IsFilled)
	{
		for (j = -y; j < y; j ++)
		{
			if (OLED_IsInAngle(0, j, StartAngle, EndAngle)) {OLED_DrawPoint(X, Y + j);}
		}
	}
	
	while (x < y)
	{
		x ++;
		if (d < 0)
		{
			d += 2 * x + 1;
		}
		else
		{
			y --;
			d += 2 * (x - y) + 1;
		}
		
		if (OLED_IsInAngle(x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + y);}
		if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
		if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
		if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
		if (OLED_IsInAngle(x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y - y);}
		if (OLED_IsInAngle(y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y - x);}
		if (OLED_IsInAngle(-x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + y);}
		if (OLED_IsInAngle(-y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + x);}
		
		if (IsFilled)
		{
			for (j = -y; j < y; j ++)
			{
				if (OLED_IsInAngle(x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + j);}
				if (OLED_IsInAngle(-x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + j);}
			}
			
			for (j = -x; j < x; j ++)
			{
				if (OLED_IsInAngle(-y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + j);}
				if (OLED_IsInAngle(y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + j);}
			}
		}
	}
}



/*****************jiangxiekeji.com*****************/
