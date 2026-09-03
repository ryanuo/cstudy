#ifndef __OLED_SHIM_H
#define __OLED_SHIM_H

#include <stdint.h>

/* 引脚定义 */
#define GPIO_PIN_8  ((uint16_t)0x0100U)
#define GPIO_PIN_9  ((uint16_t)0x0200U)
#define GPIOB       ((void *)0x40010C00U)

/* BitAction 类型 */
typedef uint8_t BitAction;

/* RCC 时钟使能 */
#define RCC_APB2Periph_GPIOB  ((uint32_t)0x00000008)
#define RCC_APB2PeriphClockCmd(x, y)  do { \
    __IO uint32_t tmpreg; \
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN); \
    tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN); \
    (void)tmpreg; \
} while(0)

/* GPIO 模式与速度 */
#define GPIO_Mode_Out_OD  ((uint8_t)0x14)
#define GPIO_Speed_50MHz  ((uint8_t)0x03)

/* SPL 风格结构体 */
typedef struct {
    uint32_t GPIO_Pin;
    uint8_t  GPIO_Speed;
    uint8_t  GPIO_Mode;
} GPIO_InitTypeDef;

/* GPIO 操作宏 */
#define GPIO_WriteBit(GPIOx, Pin, BitVal)  do { \
    (void)(GPIOx); \
    if (BitVal) { \
        *((volatile uint32_t *)((uint32_t)(GPIOx) + 0x0C)) = (uint16_t)(Pin); \
    } else { \
        *((volatile uint32_t *)((uint32_t)(GPIOx) + 0x0C)) = (uint16_t)((uint32_t)(Pin) << 16); \
    } \
} while(0)

/* GPIO 初始化 - 使用软件 I2C 模拟，不调用 HAL_GPIO_Init */
#define GPIO_Init(GPIOx, pStruct)  do { \
    (void)(GPIOx); \
    (void)(pStruct); \
} while(0)

#endif
