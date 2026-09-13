#include "artemide.h"
#include "level3.h"

// IMPROVEMENT: level 3 is a two-lane car chase. It reuses the submitted hero,
// score and lives data so the HUD and retry flow stay consistent.
static uint8_t state3;
static uint8_t lane3;
static int16_t obstacle3X;
static uint8_t obstacle3Lane;
static uint16_t frame3;
static uint8_t invuln3;
static bool scored3;

static int16_t laneY(uint8_t lane) { return lane ? 43 : 25; }

void level3Init(hero &h) {
  state3 = L3_RUNNING;
  lane3 = 1;
  obstacle3X = 150;
  obstacle3Lane = 0;
  frame3 = invuln3 = 0;
  scored3 = false;
  h.score = 0;
  h.lives = 6;
}

void level3Update(hero &h) {
  if (state3 != L3_RUNNING) return;
  ++frame3;
  if (invuln3) --invuln3;
  // IMPROVEMENT: Up and Down change lanes; Right adds chase speed.
  if (arduboy.justPressed(UP_BUTTON)) lane3 = 0;
  if (arduboy.justPressed(DOWN_BUTTON)) lane3 = 1;
  obstacle3X -= arduboy.pressed(RIGHT_BUTTON) ? 5 : 3;
  if (obstacle3X < 35 && obstacle3X > 10 && obstacle3Lane == lane3 && !invuln3) {
    if (h.lives <= 1) {
      h.lives = 0;
      state3 = L3_LOST;
      return;
    }
    --h.lives;
    invuln3 = 45;
  }
  if (!scored3 && obstacle3X < 10) {
    scored3 = true;
    if (++h.score >= L3_TARGET) state3 = L3_WON;
  }
  if (obstacle3X < -16) {
    obstacle3X = 140 + random(0, 50);
    obstacle3Lane = random(0, 2);
    scored3 = false;
  }
}

static void drawCar(int16_t x, int16_t y, bool robotCar) {
  arduboy.drawRect(x, y + 5, 24, 8, WHITE);
  arduboy.drawRect(x + 5, y, 12, 6, WHITE);
  arduboy.drawCircle(x + 5, y + 14, 3, WHITE);
  arduboy.drawCircle(x + 19, y + 14, 3, WHITE);
  if (robotCar) arduboy.drawRect(x + 9, y + 1, 4, 4, WHITE);
}

void level3Draw(hero &h) {
  // IMPROVEMENT: road markings and speed streaks communicate the driving pace.
  for (uint8_t i = 0; i < 6; ++i) {
    int16_t x = (i * 28 + 128 - (frame3 * 3) % 168) % 168 - 20;
    arduboy.drawFastHLine(x, 41, 12, WHITE);
  }
  if (arduboy.pressed(RIGHT_BUTTON)) {
    for (uint8_t i = 0; i < 4; ++i)
      arduboy.drawFastHLine((frame3 * 8 + i * 33) % 100, 15 + i * 10, 9, WHITE);
  }
  drawCar(12, laneY(lane3), true);
  arduboy.drawBitmap(obstacle3X, laneY(obstacle3Lane) + 3, comp, 8, 8, WHITE);
  arduboy.setCursor(0, 0);
  arduboy.print(F("S:")); arduboy.print(h.score); arduboy.print(F("/25"));
  for (int slot = 0; slot < 3; ++slot) {
    const uint8_t *heart = h.lives >= slot * 2 + 2 ? heart_1 :
                           h.lives >= slot * 2 + 1 ? heart_2 : heart_3;
    arduboy.drawBitmap(104 + slot * 8, 0, heart, 8, 8, WHITE);
  }
  if (invuln3 && (frame3 % 8) >= 4) arduboy.fillRect(12, laneY(lane3), 24, 18, BLACK);
}

uint8_t level3State() { return state3; }
