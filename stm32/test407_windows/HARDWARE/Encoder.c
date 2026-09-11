#include "stm32f4xx.h"

/* 旋转编码器状态 */
volatile int32_t  Encoder_Count = 0;      // 旋转计数，可正可负
volatile uint8_t  Encoder_SW_Pressed = 0; // 按键按下标志

/* ================= 编码器初始化 ================= */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1. 开时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);   // PE9/PE11/PE13
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* 2. PE9(A)、PE11(B)、PE13(SW) 上拉输入 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* 3. PE9 映射到 EXTI9（A 相） */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

    /* 4. PE9 下降沿触发（A 相下降沿读 B 判方向） */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 5. PE13 映射到 EXTI13（SW 按键） */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource13);

    EXTI_InitStructure.EXTI_Line    = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  // 按下为低
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 6. NVIC：EXTI9_5（PE9），EXTI15_10（PE13） */
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/* ================= 读取接口 ================= */
int32_t Encoder_GetCount(void)
{
    return Encoder_Count;
}

/* 计数清零（按键按下时调用） */
void Encoder_ResetCount(void)
{
    Encoder_Count = 0;
}

uint8_t Encoder_GetSW(void)
{
    return Encoder_SW_Pressed;
}

/* 清除按键按下标志，主循环处理完按键动作后调用 */
void Encoder_ClearSW(void)
{
    Encoder_SW_Pressed = 0;
}

/* ================= A 相中断：判方向 ================= */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line9) == SET)
    {
        /* A 相下降沿时读 B 相：
         * B = 0 → 逆时针
         * B = 1 → 顺时针
         * 接反了就把 +1/-1 对调
         */
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0)
        {
            Encoder_Count++;
        }
        else
        {
            Encoder_Count--;
        }

        EXTI_ClearITPendingBit(EXTI_Line9);
    }
}

/* ================= SW 按键中断 ================= */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line13) == SET)
    {
        Encoder_SW_Pressed = 1;   // 主循环看到后处理，再清零
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
}