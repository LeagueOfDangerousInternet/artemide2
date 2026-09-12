#ifndef music_h
#define music_h

#include "artemide.h"
#include "music.h"
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247

#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494

#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#ifndef NOTE_AS5
  #define NOTE_AS5 898
#endif
#define NOTE_B5  988

#define NOTE_E6  1319
#define NOTE_C6  1047
#ifndef NOTE_REST
  #define NOTE_REST 0
#endif
#define NOTE_END 0


extern const uint16_t soundJump[] PROGMEM;

extern const uint16_t soundCoin[] PROGMEM;

extern const uint16_t soundDamage[] PROGMEM;

extern const uint16_t soundLaser[] PROGMEM;

extern const uint16_t backgroundMusic[] PROGMEM;

extern const uint16_t interstellar_simple[] PROGMEM;

extern const uint16_t stayed_gone_intro[] PROGMEM;

extern const uint16_t stayed_gone_finale[] PROGMEM;

#endif
