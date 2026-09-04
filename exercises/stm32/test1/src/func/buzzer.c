#include "buzzer.h"
#include "stm32f1xx_hal.h"

#define BUZZER_PIN  GPIO_PIN_1
#define BUZZER_PORT GPIOA

void Buzzer_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = BUZZER_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BUZZER_PORT, &gpio);
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void Buzzer_Stop(void) {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void Buzzer_Start(void) {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

/* 播放一个音符。
 * freq==NOTE_REST 时只静音延时。播放中检测 PA5（模式切换键）按下
 * 则立即中断整曲返回 —— 让 main 循环能切走模式。 */
static volatile uint8_t s_abort = 0;

void Buzzer_PlayNote(uint16_t freq, uint16_t duration) {
    if (freq == NOTE_REST) {
        /* 休止符：分段延时，期间也响应按键 */
        uint32_t waited = 0;
        while (waited < duration) {
            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) {
                while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET); /* 等松手，防 main 重复触发 */
                s_abort = 1;
                return;
            }
            HAL_Delay(10);
            waited += 10;
        }
        return;
    }
    /* GPIO 翻转产生方波 */
    uint32_t half_us = 500000 / freq;
    uint32_t cycles = (uint32_t)duration * 1000 / (half_us * 2);
    for (uint32_t c = 0; c < cycles; c++) {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) {
            while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET); /* 等松手 */
            s_abort = 1;
            return;
        }
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        for (volatile uint32_t d = 0; d < half_us * 7; d++);
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
        for (volatile uint32_t d = 0; d < half_us * 7; d++);
    }
}

/* 播放歌曲 */
void Buzzer_PlayMusic(const MusicNote_t *music, uint16_t length, uint16_t beat_ms) {
    for (uint16_t i = 0; i < length; i++) {
        if (s_abort) break;                       /* 被按键中断 */
        uint32_t duration = ((uint32_t)beat_ms * music[i].beat) / 4;
        Buzzer_PlayNote(music[i].freq, duration);
    }
    s_abort = 0;                                  /* 播完/中断都复位，下次从头播 */
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

static const MusicNote_t song_qinghuaci[] = {
    /* 天青色等烟雨 而我在等你 */
    {NOTE_M4, 4}, {NOTE_M5, 4}, {NOTE_M6, 4}, {NOTE_M5, 4}, {NOTE_M4, 4}, {NOTE_M4, 8}, {NOTE_REST, 2},
    {NOTE_M1, 4}, {NOTE_M2, 4}, {NOTE_M4, 4}, {NOTE_M4, 4}, {NOTE_M1, 12}, {NOTE_REST, 4},
    /* 炊烟袅袅升起 隔江千万里 */
    {NOTE_M4, 4}, {NOTE_M5, 4}, {NOTE_M6, 4}, {NOTE_M5, 4}, {NOTE_M4, 4}, {NOTE_M4, 4},
    {NOTE_M5, 4}, {NOTE_M4, 2}, {NOTE_M2, 2}, {NOTE_M1, 4}, {NOTE_M2, 12}, {NOTE_REST, 4},
    /* 在瓶底书汉隶 仿前朝的飘逸 */
    {NOTE_M4, 4}, {NOTE_M5, 4}, {NOTE_M6, 4}, {NOTE_M5, 4}, {NOTE_M4, 4}, {NOTE_M4, 8}, {NOTE_REST, 2},
    {NOTE_M1, 4}, {NOTE_M2, 4}, {NOTE_M4, 4}, {NOTE_M4, 4}, {NOTE_M1, 12}, {NOTE_REST, 4},
    /* 就当我为遇见你伏笔 */
    {NOTE_M4, 4}, {NOTE_M5, 4}, {NOTE_M4, 4}, {NOTE_M2, 4}, {NOTE_M1, 4}, {NOTE_M2, 4},
    {NOTE_M1, 4}, {NOTE_L6, 8}, {NOTE_M1, 16},
};

void Buzzer_Play(uint8_t song_id) {
    switch (song_id) {
        case 1:
            Buzzer_PlayMusic(song_qifengle, sizeof(song_qifengle) / sizeof(song_qifengle[0]), 500);
            break;
        case 2:
            Buzzer_PlayMusic(song_qinghuaci, sizeof(song_qinghuaci) / sizeof(song_qinghuaci[0]), 500);
            break;
        default:
            break;
    }
}
