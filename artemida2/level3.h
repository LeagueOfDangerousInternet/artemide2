#ifndef LEVEL3_H
#define LEVEL3_H

#include <Arduboy2.h>
#include "hero.h"

#define L3_GROUND    56
#define L3_PLAYER_X  16
#define L3_HERO_W    16
#define L3_HERO_H    16

#define L3_CAT_HP    5

#define L3_RUNNING   0
#define L3_WON       1
#define L3_LOST      2

void level3Init(hero &h);
void level3Update(hero &h);
void level3Draw(hero &h);
uint8_t level3State();

#endif
