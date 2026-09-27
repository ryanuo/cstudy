#include "stm32f4xx.h"

/* 旋转编码器状态 */
volatile int32_t  Encoder_Count = 0;      // 旋转计数，可正可负
volatile uint8_t  Encoder_SW_Pressed = 0; // 按键按下标志

/* 引脚映射
 * A/B 原在 PE9/PE11，与板上 TFTLCD 的 FSMC_D6/D8 共用同一条数据总线，已挪到 PE5/PE6；
 * PE5 与 PE6 同属 EXTI9_5 组，A 相触发中断，B 相在中断里读电平。
 * 注意：SW 仍在 PE13（= LCD 的 FSMC_D10，也是 LED3），若要用屏需一并挪走。
 */
#define ENC_A_PIN        GPIO_Pin_5
#define ENC_B_PIN        GPIO_Pin_6
#define ENC_SW_PIN       GPIO_Pin_13
#define ENC_A_PINSOURCE  EXTI_PinSource5
#define ENC_A_EXTI_LINE  EXTI_Line5

/* ================= 编码器初始化 ================= */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1. 开时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);   // PE5/PE6/PE13
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* 2. PE5(A)、PE6(B)、PE13(SW) 上拉输入 */
    GPIO_InitStructure.GPIO_Pin   = ENC_A_PIN | ENC_B_PIN | ENC_SW_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* 3. PE5 映射到 EXTI5（A 相，属 EXTI9_5 组） */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, ENC_A_PINSOURCE);

    /* 4. PE5 下降沿触发（A 相下降沿读 B 判方向） */
    EXTI_InitStructure.EXTI_Line    = ENC_A_EXTI_LINE;
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

    /* 6. NVIC：EXTI9_5（PE5=A），EXTI15_10（PE13=SW） */
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
    if (EXTI_GetITStatus(ENC_A_EXTI_LINE) == SET)
    {
        /* A 相下降沿时读 B 相：
         * B = 0 → 逆时针
         * B = 1 → 顺时针
         * 接反了就把 +1/-1 对调
         */
        if (GPIO_ReadInputDataBit(GPIOE, ENC_B_PIN) == 0)
        {
            Encoder_Count++;
        }
        else
        {
            Encoder_Count--;
        }

        EXTI_ClearITPendingBit(ENC_A_EXTI_LINE);
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