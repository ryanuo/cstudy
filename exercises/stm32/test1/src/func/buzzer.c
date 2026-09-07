#include "buzzer.h"
#include "stm32f1xx_hal.h"

#define BUZZER_PIN  GPIO_PIN_6
#define BUZZER_PORT GPIOA

TIM_HandleTypeDef htim3;

void Buzzer_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* PA6 = TIM3_CH1 复用推挽输出 */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = BUZZER_PIN;
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BUZZER_PORT, &gpio);

    /* TIM3 配置：1MHz 计数时钟，PWM 模式 1，低电平有效（低电平=响）
     * OCPolarity=LOW: 计数器<CCR 时输出 LOW，计数器>=CCR 时输出 HIGH
     * 方波：CCR = ARR/2
     * 静音：CCR = 0（输出恒为 HIGH） */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;       /* 72MHz / 72 = 1MHz */
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xFFFF;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);

    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 0;                    /* 初始 CCR=0，输出 HIGH（静音） */
    oc.OCPolarity = TIM_OCPOLARITY_LOW;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void Buzzer_Stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);  /* CCR=0 → 恒 HIGH → 静音 */
}

void Buzzer_Start(void)
{
    /* 旧接口，TIM 模式下不使用 */
}

/* 内部：设置频率（0=静音） */
static void Buzzer_SetFreq(uint16_t freq)
{
    if (freq == 0) {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    } else {
        uint16_t arr = (1000000 / freq) - 1;       /* 1MHz / freq - 1 */
        __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, arr / 2);  /* 50% 占空比方波 */
    }
}

/* ===== 非阻塞音乐播放（状态机，1ms 节拍） ===== */
static const MusicNote_t *s_music = NULL;
static uint16_t s_length = 0;
static uint16_t s_beat_ms = 0;
static uint16_t s_idx = 0;
static uint16_t s_note_ms = 0;
static uint16_t s_note_dur = 0;
static uint8_t  s_playing = 0;
static uint8_t  s_paused = 0;
static uint16_t s_current_arr = 0;  /* 当前音符的 ARR，用于暂停恢复 */

void Music_Play(const MusicNote_t *music, uint16_t length, uint16_t beat_ms)
{
    s_music = music;
    s_length = length;
    s_beat_ms = beat_ms;
    s_idx = 0;
    s_playing = 1;
    s_paused = 0;
    s_note_ms = 0;
    s_note_dur = 0;
    s_current_arr = 0;
}

void Music_Toggle(void)
{
    if (!s_playing) return;
    s_paused = !s_paused;
    if (s_paused) {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);  /* 静音 */
    } else {
        if (s_current_arr > 0) {
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, s_current_arr / 2);
        }
    }
}

uint8_t Music_IsPlaying(void)
{
    return s_playing && !s_paused;
}

void Music_Update(void)
{
    if (!s_playing || s_paused) return;

    if (s_idx >= s_length) {
        s_playing = 0;
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
        return;
    }

    uint16_t freq = s_music[s_idx].freq;
    uint16_t beat = s_music[s_idx].beat;

    if (s_note_ms == 0) {
        /* 新音符开始 */
        s_note_dur = (uint32_t)s_beat_ms * beat / 4;
        if (freq == 0) {
            /* 休止符 */
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
            s_current_arr = 0;
        } else {
            s_current_arr = (1000000 / freq) - 1;
            __HAL_TIM_SET_AUTORELOAD(&htim3, s_current_arr);
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, s_current_arr / 2);
        }
    }

    s_note_ms++;

    if (s_note_ms >= s_note_dur) {
        s_idx++;
        s_note_ms = 0;
    }
}

/* 起风了 乐谱 */
static const MusicNote_t song_qifengle[] = {
    {NOTE_M7, 1}, {NOTE_H1, 1}, {NOTE_H2, 1}, {NOTE_H3, 1},
    {NOTE_M3, 2}, {NOTE_H5, 1}, {NOTE_H3, 1}, {NOTE_H3, 2},
    {NOTE_REST, 4},
    {NOTE_M7, 1}, {NOTE_H1, 1}, {NOTE_H2, 1}, {NOTE_H3, 1},
    {NOTE_M2, 2}, {NOTE_H5, 1}, {NOTE_H3, 1}, {NOTE_H2, 1},
    {NOTE_H3, 1}, {NOTE_H1, 1}, {NOTE_H2, 1}, {NOTE_M7, 1},
    {NOTE_H1, 1}, {NOTE_M5, 1}, {NOTE_REST, 4}, {NOTE_REST, 4},
    {NOTE_M7, 1}, {NOTE_H1, 1}, {NOTE_H2, 1}, {NOTE_H3, 1},
    {NOTE_M3, 2}, {NOTE_H5, 1}, {NOTE_H3, 1}, {NOTE_H3, 2},
    {NOTE_REST, 4},
    {NOTE_M7, 1}, {NOTE_H1, 1}, {NOTE_H2, 1}, {NOTE_H3, 1},
    {NOTE_M2, 2}, {NOTE_H5, 1}, {NOTE_H3, 1}, {NOTE_H2, 1},
    {NOTE_H3, 1}, {NOTE_H1, 1}, {NOTE_H2, 1}, {NOTE_M7, 1},
    {NOTE_H1, 1}, {NOTE_M5, 1}, {NOTE_REST, 4}, {NOTE_REST, 4},
    {NOTE_M2, 4}, {NOTE_M1, 1}, {NOTE_M2, 4}, {NOTE_M1, 1},
    {NOTE_M2, 2}, {NOTE_M3, 2}, {NOTE_M5, 2}, {NOTE_M3, 2},
    {NOTE_REST, 4},
    {NOTE_M2, 4}, {NOTE_M1, 1}, {NOTE_M2, 4}, {NOTE_M1, 1},
    {NOTE_M2, 1}, {NOTE_M3, 1}, {NOTE_M2, 1}, {NOTE_M1, 1},
    {NOTE_M1, 2}, {NOTE_REST, 4}, {NOTE_REST, 4},
    {NOTE_M2, 4}, {NOTE_M1, 1}, {NOTE_M2, 4}, {NOTE_M1, 1},
    {NOTE_M2, 2}, {NOTE_M3, 2}, {NOTE_M5, 2}, {NOTE_M3, 2},
    {NOTE_REST, 4},
    {NOTE_M2, 4}, {NOTE_M3, 1}, {NOTE_M2, 2}, {NOTE_M1, 1},
    {NOTE_M2, 1}, {NOTE_M2, 4}, {NOTE_REST, 4}, {NOTE_REST, 4},
    {NOTE_M2, 4}, {NOTE_M1, 1}, {NOTE_M2, 4}, {NOTE_M1, 1},
    {NOTE_M2, 2}, {NOTE_M3, 2}, {NOTE_M5, 2}, {NOTE_M3, 2},
    {NOTE_REST, 4},
    {NOTE_M2, 4}, {NOTE_M3, 1}, {NOTE_M2, 2}, {NOTE_M1, 1},
    {NOTE_L6, 4}, {NOTE_REST, 4},
    {NOTE_M3, 1}, {NOTE_M2, 1}, {NOTE_M1, 1}, {NOTE_M2, 1},
    {NOTE_REST, 4}, {NOTE_M1, 2}, {NOTE_REST, 4},
    {NOTE_M3, 1}, {NOTE_M2, 1}, {NOTE_M1, 1}, {NOTE_M2, 1},
    {NOTE_M1, 2}, {NOTE_M3, 1}, {NOTE_M2, 1}, {NOTE_M1, 1}, {NOTE_M2, 1},
    {NOTE_M1, 2},
    {NOTE_M1, 2}, {NOTE_M2, 2}, {NOTE_M3, 2}, {NOTE_M1, 2},
    {NOTE_M6, 2}, {NOTE_M5, 1}, {NOTE_M6, 2}, {NOTE_REST, 4},
    {NOTE_M1, 1}, {NOTE_M7, 2}, {NOTE_M6, 1}, {NOTE_M7, 4},
    {NOTE_REST, 4}, {NOTE_REST, 4},
    {NOTE_M7, 2}, {NOTE_M6, 1}, {NOTE_M7, 2}, {NOTE_M3, 2},
    {NOTE_M1, 1}, {NOTE_M2, 1}, {NOTE_M1, 1}, {NOTE_M7, 1},
    {NOTE_M6, 2}, {NOTE_REST, 4},
    {NOTE_M5, 2}, {NOTE_M6, 2}, {NOTE_M5, 1}, {NOTE_M6, 2},
    {NOTE_M5, 1}, {NOTE_M6, 1}, {NOTE_M5, 1}, {NOTE_M6, 2},
    {NOTE_M5, 1}, {NOTE_M2, 2}, {NOTE_M5, 1}, {NOTE_REST, 4},
    {NOTE_M5, 1}, {NOTE_M3, 2}, {NOTE_M3, 2},
    {NOTE_M1, 2}, {NOTE_M2, 2}, {NOTE_M3, 2}, {NOTE_M1, 2},
    {NOTE_M6, 2}, {NOTE_M5, 1}, {NOTE_M6, 4}, {NOTE_REST, 4},
    {NOTE_M1, 1}, {NOTE_M7, 2}, {NOTE_M6, 1}, {NOTE_M7, 4},
    {NOTE_REST, 4}, {NOTE_M7, 2}, {NOTE_M6, 1}, {NOTE_M7, 4},
    {NOTE_M3, 2}, {NOTE_H1, 1}, {NOTE_H2, 1}, {NOTE_H1, 1}, {NOTE_M7, 1},
    {NOTE_M6, 1},
    {NOTE_M5, 2}, {NOTE_M6, 2}, {NOTE_H3, 1}, {NOTE_H3, 4},
    {NOTE_M5, 2}, {NOTE_M6, 2}, {NOTE_H3, 1}, {NOTE_H3, 2},
    {NOTE_M5, 1}, {NOTE_M6, 8},
    {NOTE_H1, 2}, {NOTE_H2, 2}, {NOTE_H3, 2}, {NOTE_H6, 1},
    {NOTE_H5, 2}, {NOTE_H6, 1}, {NOTE_H5, 2}, {NOTE_H6, 1},
    {NOTE_H5, 4}, {NOTE_H2, 1}, {NOTE_H3, 4}, {NOTE_H6, 1},
    {NOTE_H5, 4}, {NOTE_H6, 1}, {NOTE_H5, 4}, {NOTE_H6, 1},
    {NOTE_H5, 2}, {NOTE_H3, 1}, {NOTE_H3, 4}, {NOTE_REST, 4},
    {NOTE_H2, 1}, {NOTE_H2, 1}, {NOTE_H1, 1}, {NOTE_M6, 2},
    {NOTE_H1, 2}, {NOTE_H1, 1}, {NOTE_H2, 1}, {NOTE_H2, 1}, {NOTE_H1, 1}, {NOTE_M6, 1},
    {NOTE_H1, 2}, {NOTE_H3, 6}, {NOTE_H3, 1}, {NOTE_H2, 1},
    {NOTE_H3, 1}, {NOTE_H2, 2}, {NOTE_REST, 4},
    {NOTE_H1, 2}, {NOTE_H2, 2}, {NOTE_H3, 2}, {NOTE_H6, 1},
    {NOTE_H5, 4}, {NOTE_H6, 1}, {NOTE_H5, 4}, {NOTE_H6, 1},
    {NOTE_H5, 2}, {NOTE_REST, 4},
    {NOTE_H2, 1}, {NOTE_H3, 2}, {NOTE_H6, 1}, {NOTE_H5, 4},
    {NOTE_H6, 1}, {NOTE_H5, 4}, {NOTE_H6, 1}, {NOTE_H5, 2},
    {NOTE_H3, 2}, {NOTE_REST, 4},
    {NOTE_H3, 6}, {NOTE_H2, 2}, {NOTE_H1, 1}, {NOTE_M6, 2},
    {NOTE_H3, 2}, {NOTE_H2, 2}, {NOTE_H1, 1}, {NOTE_M6, 2},
    {NOTE_M6, 1}, {NOTE_H1, 1}, {NOTE_H1, 4},
};

/* 兼容旧接口 */
void Buzzer_Play(uint8_t song_id)
{
    if (song_id == 1) {
        Music_Play(song_qifengle, sizeof(song_qifengle) / sizeof(MusicNote_t), 500);
    }
}
