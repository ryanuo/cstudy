// #include "stm32f4xx.h"

// /* 外部中断：PA8 -> EXTI8 */

// volatile uint16_t CountSensor_value = 0;
// void CountSensor_Init(void)
// {
//     GPIO_InitTypeDef GPIO_InitStructure;
//     EXTI_InitTypeDef EXTI_InitStructure;
//     NVIC_InitTypeDef NVIC_InitStructure;

//     /* 1. 开时钟（F4 关键差异） */
//     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // GPIOA 在 AHB1
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // F4 用 SYSCFG 代替 AFIO

//     /* 2. PA8 上拉输入 */
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//     GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // F4 用 PuPd 指定上下拉
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // F4 最高 100MHz
//     GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 输入模式也要写，避免垃圾值
//     GPIO_Init(GPIOA, &GPIO_InitStructure);

//     /* 3. 映射 PA8 到 EXTI8（F4 用 SYSCFG） */
//     SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);

//     /* 4. EXTI8 下降沿触发 */
//     EXTI_InitStructure.EXTI_Line = EXTI_Line8;
//     EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//     EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//     EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//     EXTI_Init(&EXTI_InitStructure);

//     /* 5. NVIC：EXTI8 属于 EXTI9_5_IRQn */
//     NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);
// }

// uint32_t CountSensor_GetValue(void)
// {
//     return CountSensor_value;
// }

// void EXTI9_5_IRQHandler(void)
// {
//     /* 粗延时，约几 ms，具体时长靠试 */
//     for (volatile uint32_t i = 0; i < 200000; i++)
//         ;
//     if (EXTI_GetITStatus(EXTI_Line8) == SET)
//     {
//         CountSensor_value++;
//         EXTI_ClearITPendingBit(EXTI_Line8);
//     }
// }