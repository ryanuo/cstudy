#ifndef __BUZZER_H
#define __BUZZER_H

#include <stdint.h>

#define NOTE_L1     262
#define NOTE_L2     294
#define NOTE_L3     330
#define NOTE_L4     349
#define NOTE_L5     392
#define NOTE_L6     440
#define NOTE_L7     494

#define NOTE_M1     523
#define NOTE_M2     587
#define NOTE_M3     659
#define NOTE_M4     698
#define NOTE_M5     784
#define NOTE_M6     880
#define NOTE_M7     988

#define NOTE_H1     1047
#define NOTE_H2     1175
#define NOTE_H3     1319
#define NOTE_H4     1397
#define NOTE_H5     1568
#define NOTE_H6     1760
#define NOTE_H7     1976

#define NOTE_REST   0

typedef struct {
    uint16_t freq;
    uint16_t beat;
} MusicNote_t;

void Buzzer_Init(void);
void Buzzer_Stop(void);
void Buzzer_PlayNote(uint16_t freq, uint16_t duration);
void Buzzer_PlayMusic(const MusicNote_t *music, uint16_t length, uint16_t beat_ms);

void Buzzer_Play(uint8_t song_id);

#endif