#ifndef LEVEL3_H
#define LEVEL3_H

#include "hero.h"

// IMPROVEMENT: compact driving finale added after the two submitted levels.
#define L3_RUNNING 0
#define L3_WON 1
#define L3_LOST 2
#define L3_TARGET 25

void level3Init(hero &h);
void level3Update(hero &h);
void level3Draw(hero &h);
uint8_t level3State();

#endif
