#ifndef LEVEL2_H
#define LEVEL2_H

#include <Arduboy2.h>
#include "hero.h"

// Раскладка
#define L2_GROUND 56
#define L2_PLAYER_X 20
#define L2_HERO_W 16
#define L2_HERO_H 16


#define L2_BUG 0
#define L2_CLOUD 1
#define L2_ROCKET 2

// Состояния уровня
#define L2_RUNNING 0
#define L2_WON 1
#define L2_LOST 2

// Цель
#define L2_TARGET 25

void level2Init(hero &h);
void level2Update(hero &h);
void level2Draw(hero &h);
uint8_t level2State();

#endif
