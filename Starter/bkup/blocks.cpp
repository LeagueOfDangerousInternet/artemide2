#include "artemide.h"
#include <avr/pgmspace.h>


// Простой квадрат 8x8 с рамкой
const uint8_t block[] PROGMEM = {
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111
};


const uint8_t space[] PROGMEM = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};
