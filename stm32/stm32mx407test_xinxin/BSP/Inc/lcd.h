#ifndef __LCD_H
#define __LCD_H

#include "main.h" // CubeMX 生成的，包含 stm32f4xx_hal.h 和 u8/u16/u32 类型

/* ================= LCD FSMC 地址映射 =================
 * CubeMX 配置：Bank1 NORSRAM4，基址 0x6C000000，A12 作为 RS(数据/命令区分线)
 * 2.8 寸 ILI9341 / 7 寸 SSD1963 模块：CMD = A12 = 0，DATA = A12 = 1
 * （7 寸 RA8875 模块正好相反：CMD = A12 = 1，DATA = A12 = 0，不要混用）*/
#define LCD_BASE ((uint32_t)0x6C000000)
#define LCD_CMD (*(__IO uint16_t *)(LCD_BASE))
#define LCD_DATA (*(__IO uint16_t *)(LCD_BASE + 0x2000))

/* ================= 背光 =================
 * 板上 LCD_BL 接 PF10（原理图 MCU 第 22 脚），经模块上的三极管开关背光 LED，
 * 必须拉高背光才亮。原厂 23 个例程都是 LCD_BACK = PFout(10) 后置 1 */
#define LCD_BACK_GPIO_PORT GPIOF
#define LCD_BACK_GPIO_PIN GPIO_PIN_10
#define LCD_BACK_SET(x)                                                        \
  HAL_GPIO_WritePin(LCD_BACK_GPIO_PORT, LCD_BACK_GPIO_PIN,                    \
                    (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/* ================= 颜色定义 ================= */
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40
#define BRRED 0XFC07
#define GRAY 0X8430

/* ================= 扫描方向定义 ================= */
#define L2R_U2D 0 // 从左到右,从上到下
#define L2R_D2U 1 // 从左到右,从下到上
#define R2L_U2D 2 // 从右到左,从上到下
#define R2L_D2U 3 // 从右到左,从下到上
#define U2D_L2R 4 // 从上到下,从左到右
#define U2D_R2L 5 // 从上到下,从右到左
#define D2U_L2R 6 // 从下到上,从左到右
#define D2U_R2L 7 // 从下到上,从右到左

/* ================= 全局变量 ================= */
extern u16 lcd_id;       // LCD ID
extern u8 dir_flag;      // 横竖屏控制：0 竖屏，1 横屏
extern u16 lcd_width;    // LCD 宽度
extern u16 lcd_height;   // LCD 高度
extern u16 write_gramcmd; // 写 GRAM 指令
extern u16 setxcmd;       // 设置 X 坐标指令
extern u16 setycmd;       // 设置 Y 坐标指令
extern u16 BRUSH_COLOR;   // 画笔颜色
extern u16 BACK_COLOR;    // 背景颜色

/* ================= 函数声明 ================= */
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_Value);
u16 LCD_ReadReg(u16 LCD_Reg);
void LCD_WriteGRAM(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Open_Window(u16 X0, u16 Y0, u16 width, u16 height);
void Set_Scan_Direction(u8 direction);
void Set_Display_Mode(u8 mode);
void LCD_SetCursor(u16 Xaddr, u16 Yaddr);
u16 LCD_GetPoint(u16 x, u16 y);
void LCD_DrawPoint(u16 x, u16 y);
void LCD_Color_DrawPoint(u16 x, u16 y, u16 color);
void Ssd1963_Set_BackLight(u8 BL_value);
void LCD_Init(void);
void Text_Foreground_Color(u16 Color);
void Text_Background_Color(u16 Color);
void LCD_Clear(u16 color);
void LCD_Fill_onecolor(u16 sx, u16 sy, u16 ex, u16 ey, u16 color);
void LCD_Draw_Picture(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color);
void LCD_Draw_Line(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Draw_Rectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r);
void LCD_DisplayChar(u16 x, u16 y, u8 word, u8 size);
void LCD_DisplayString(u16 x, u16 y, u8 size, u8 *p);
void LCD_DisplayString_color(u16 x, u16 y, u8 size, u8 *p, u16 brushcolor,
                             u16 backcolor);
u32 Counter_Power(u8 a, u8 n);
void LCD_DisplayNum(u16 x, u16 y, u32 value, u8 len, u8 size, u8 mode);
void LCD_DisplayNum_color(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode,
                          u16 brushcolor, u16 backcolor);

#endif
