#include "artemide.h"
#include "level3.h"
#include "cat.h"


static int16_t catX;
static int16_t catY;
static uint8_t catHp;
static uint8_t catState;         // 0 idle, 1 dodge, 2 attack
static uint8_t catStateTimer;
static uint8_t catHopTimer;

static uint8_t state3;
static uint8_t invuln3;
static uint16_t frame3;

static uint8_t projActive;
static int16_t projX;
static int16_t projY;
static int16_t projVx;

static uint8_t playerVx;
static uint8_t attackTimer;
static uint8_t attackHitDone;


static void drawHeartProjectile(int16_t x, int16_t y) {
  arduboy.drawBitmap(x, y, heart_1, 8, 8, WHITE);
}


static void drawCat(int16_t x, int16_t y) {
  arduboy.drawBitmap(x, y, space_cat, 16, 16, WHITE);
}


void level3Init(hero &h) {
  catX = 100;
  catY = L3_GROUND - 16;
  catHp = L3_CAT_HP;
  catState = 0;
  catStateTimer = 0;
  catHopTimer = 0;

  state3 = L3_RUNNING;
  invuln3 = 0;
  frame3 = 0;

  projActive = 0;
  projX = 0;
  projY = 0;
  projVx = 4;

  playerVx = 1;
  attackTimer = 0;
  attackHitDone = 0;

  h.x = L3_PLAYER_X;
  h.y = L3_GROUND - L3_HERO_H;
  h.vx = 0;
  h.vy = 0;
  h.status = 0;
  h.score = 0;
  h.lives = 6;
}


void level3Update(hero &h) {
  if (state3 == L3_WON || state3 == L3_LOST) {
    return;
  }

  ++frame3;
  if (invuln3 > 0) {
    invuln3--;
  }

  // Присед
  if (arduboy.pressed(DOWN_BUTTON)) {
    h.status |= 1;
  }
  else {
    h.status &= ~1;
  }

  // Движение героя влево/вправо
  int16_t px = h.x;
  if (arduboy.pressed(RIGHT_BUTTON)) {
    px += playerVx;
  }
  if (arduboy.pressed(LEFT_BUTTON)) {
    px -= playerVx;
  }
  if (px < 8) {
    px = 8;
  }
  if (px > 80) {
    px = 80;
  }
  h.x = (uint8_t)px;

  // Выстрел
  if (arduboy.justPressed(A_BUTTON) && !projActive) {
    projActive = 1;
    projX = h.x + 14;
    projY = h.y + 6;
  }

  // Полёт снаряда
  if (projActive) {
    projX += projVx;
    if (projX > 128) {
      projActive = 0;
    }
  }

  // --- AI кота ---
  if (catState == 0) {
    // Уворот: если снаряд близко и летит справа налево
    if (projActive && projX > catX - 44 && projX < catX + 4) {
      if (random(0, 100) < 55) {
        projActive = 0;
        catState = 1;
        catStateTimer = 12;
        if (catY > L3_GROUND - 20) {
          catY = L3_GROUND - 30;
        }
        else {
          catY = L3_GROUND - 16;
        }
      }
    }

    // Медленный дрейф кота по X
    if (frame3 % 3 == 0) {
      if (catX < 116) {
        catX += 1;
      }
    }

    // Прыжки без причины
    if (catHopTimer > 0) {
      catHopTimer--;
    }
    else {
      catHopTimer = 40 + random(0, 40);
      if (catY > L3_GROUND - 20) {
        catY = L3_GROUND - 26;
      }
      else {
        catY = L3_GROUND - 16;
      }
    }

    // Атака: игрок слишком близко
    if (h.x + L3_HERO_W + 6 >= catX) {
      catState = 2;
      catStateTimer = 40;
      attackTimer = 20;
      attackHitDone = 0;
    }
  }
  else if (catState == 1) {
    if (catStateTimer > 0) {
      catStateTimer--;
    }
    else {
      catState = 0;
    }
  }
  else if (catState == 2) {
    if (catStateTimer > 0) {
      catStateTimer--;
    }
    if (attackTimer > 0) {
      attackTimer--;
    }

    // Момент удара
    if (attackTimer == 0 && !attackHitDone) {
      attackHitDone = 1;
      if (invuln3 == 0 && !(h.status & 1)) {
        if (h.lives <= 2) {
          h.lives = 0;
          state3 = L3_LOST;
          return;
        }
        h.lives -= 2;
        invuln3 = 60;
      }
    }

    if (catStateTimer == 0) {
      catX = 100 + random(0, 16);
      catY = L3_GROUND - 16;
      catState = 0;
      h.x = L3_PLAYER_X;
    }
  }

  // --- Столкновение снаряда и кота ---
  if (projActive) {
    bool overlapX = projX + 8 > catX + 3 && projX < catX + 13;
    bool overlapY = projY + 6 > catY + 4 && projY < catY + 14;
    if (overlapX && overlapY) {
      projActive = 0;
      if (catHp > 0) {
        catHp--;
      }
      // Отброс
      catX += 6;
      if (catX > 116) {
        catX = 116;
      }
      if (catHp == 0) {
        state3 = L3_WON;
        h.score = L3_CAT_HP;
        return;
      }
    }
  }
}


void level3Draw(hero &h) {
  // Звёзды
  for (uint8_t i = 0; i < 9; ++i) {
    uint8_t sx = (i * 29 + 128 - (frame3 / 3) % 128) % 128;
    arduboy.drawPixel(sx, 10 + (i * 7) % 20);
  }

  // Земля
  arduboy.drawFastHLine(0, L3_GROUND, 128);

  // Кот
  bool catBlink = (catState == 2) && ((frame3 % 6) < 3);
  if (!catBlink) {
    drawCat(catX, catY);
  }

  // Снаряд
  if (projActive) {
    drawHeartProjectile(projX, projY);
  }

  // Герой
  bool blink = (invuln3 > 0) && ((time_buf % 8) >= 4);
  if (!blink) {
    const uint8_t *robot;
    if (h.status & 1) {
      robot = man_3;
    }
    else if ((frame3 / 6) % 2 == 0) {
      robot = man_1;
    }
    else {
      robot = man_2;
    }
    arduboy.drawBitmap(h.x, h.y, robot, 16, 16, WHITE);
  }

  // HUD
  arduboy.setCursor(0, 0);
  arduboy.print(F("CAT:"));
  arduboy.print(catHp);
  arduboy.print(F("/"));
  arduboy.print(L3_CAT_HP);

  for (int slot = 0; slot < 3; slot++) {
    int hx = 104 + slot * 8;
    uint8_t need = (uint8_t)(slot * 2);
    const uint8_t *sprite = heart_3;
    if (h.lives >= need + 2) {
      sprite = heart_1;
    }
    else if (h.lives >= need + 1) {
      sprite = heart_2;
    }
    arduboy.drawBitmap(hx, 0, sprite, 8, 8, WHITE);
  }

  if (state3 == L3_LOST) {
    arduboy.fillRect(20, 24, 88, 14, BLACK);
    arduboy.drawRect(20, 24, 88, 14, WHITE);
    arduboy.setCursor(37, 28);
    arduboy.print(F("GAME OVER"));
  }
  else if (state3 == L3_WON) {
    arduboy.fillRect(20, 24, 88, 14, BLACK);
    arduboy.drawRect(20, 24, 88, 14, WHITE);
    arduboy.setCursor(40, 28);
    arduboy.print(F("YOU WIN!"));
  }
}


uint8_t level3State() {
  return state3;
}
