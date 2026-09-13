#ifndef hero_h
#define hero_h

#include "artemide.h"


class hero {
public:
  uint8_t x = 0;
  int16_t y = 0;
  int8_t vx = 0;
  int8_t vy = 0;
  uint8_t lives = 6;

  uint8_t status = 0;
  uint8_t score = 0;

  hero();
  //void vel_x();
  //void vel_y();
  //void down();
  void update(uint8_t status);
  void draw();
};

#endif
