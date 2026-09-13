#ifndef music_h
#define music_h


#include "artemide.h"



#define C3  131
#define CS3 139
#define D3  147
#define DS3 156
#define E3  165
#define F3  175
#define FS3 185
#define G3  196
#define GS3 208
#define A3  220
#define AS3 233
#define B3  247

// Четвертая октава (для фоновой музыки)
#define C4  262
#define CS4 277
#define D4  294
#define DS4 311
#define E4  330
#define F4  349
#define FS4 370
#define G4  392
#define GS4 415
#define A4  440
#define AS4 466
#define B4  494

// Пятая октава (для фоновой музыки и прыжка)
#define C5  523
#define CS5 554
#define D5  587
#define DS5 622
#define E5  659
#define F5  698
#define FS5 740
#define G5  784
#define GS5 831
#define A5  880
#define AS5 932
#define B5  988

// Шестая октава (для эффектов лазера и монетки)
#define C6  1047
#define CS6 1109
#define D6  1175
#define DS6 1245
#define E6  1319
#define F6  1397
#define FS6 1480
#define G6  1568
#define GS6 1661
#define A6  1760
#define AS6 1865
#define B6  1976


extern const uint16_t soundJump[] PROGMEM;

extern const uint16_t soundCoin[] PROGMEM;

extern const uint16_t soundDamage[] PROGMEM;

extern const uint16_t soundLaser[] PROGMEM;

extern const uint16_t backgroundMusic[] PROGMEM;
extern const uint16_t interstellar[] PROGMEM;


#endif
