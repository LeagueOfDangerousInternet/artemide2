#ifndef LEVEL1_H
#define LEVEL1_H

#include <Arduboy2.h>
#include "hero.h"

#define STRIP_TILES 128
#define TILE_SIZE 8
#define STRIP_PIXELS (STRIP_TILES * TILE_SIZE)

#define HERO_W 16
#define HERO_H 16
#define HERO_SCREEN_X 30
#define FALL_Y 72          // если y больше — упал в яму

#define PIT 255            // маркер ямы

void level1Init(hero &h);
void level1Update(hero &h);
void level1Draw(hero &h);
uint8_t level1State();

#endif
