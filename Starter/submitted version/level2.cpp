#include "artemide.h"
#include "level2.h"


static int16_t obstacleX;
static uint8_t hazard;
static int16_t heightQ;      // высота прыжка в 1/4 пикселя
static int16_t velocityQ;
static uint16_t runFrame;
static uint8_t state2;
static uint8_t scored;
static uint8_t invuln2;
static uint16_t moveRemainder;


static const uint8_t *hazardSprite(uint8_t hz) {
  if (hz == L2_BUG) {
    return bug_alien;
  }
  if (hz == L2_CLOUD) {
    return dead_pixel_cloud;
  }
  return broken_rocket;
}


static int16_t obstacleY(uint8_t hz) {
  // Низ облака — на 1 px выше антенны сидящего робота.
  if (hz == L2_CLOUD) {
    return 28;
  }
  return L2_GROUND - 16;
}


static uint16_t speedQ() {
  // 6 пикселей за кадр в 1/256-х долях пикселя.
  return 512;
}


void level2Init(hero &h) {
  obstacleX = 140;
  hazard = L2_BUG;
  heightQ = 0;
  velocityQ = 0;
  runFrame = 0;
  state2 = L2_RUNNING;
  scored = 0;
  invuln2 = 0;
  moveRemainder = 0;

  h.x = L2_PLAYER_X;
  h.y = L2_GROUND - L2_HERO_H;
  h.vx = 0;
  h.vy = 0;
  h.status = 0;
  h.score = 0;
  h.lives = 6;
}


void level2Update(hero &h) {
  if (state2 == L2_WON || state2 == L2_LOST) {
    return;
  }

  ++runFrame;
  if (invuln2 > 0) {
    invuln2--;
  }


  bool onGround = (heightQ == 0);

  // Сидим: на земле и зажат A или DOWN.
  bool sit = onGround && (arduboy.pressed(A_BUTTON) || (h.status & 1));
  if (sit) {
    h.status |= 1;
  }
  else {
    h.status &= ~1;
  }

  // Прыжок: на земле, не сидим, B или UP только что нажаты.
  if (onGround && !sit && (arduboy.justPressed(B_BUTTON) || (h.status & 2))) {
    velocityQ = 17;
  }

  // Баллистика в 1/4 пикселя.
  if (velocityQ != 0 || heightQ > 0) {
    heightQ += velocityQ;
    --velocityQ;
    if (heightQ <= 0) {
      heightQ = 0;
      velocityQ = 0;
    }
  }

  // Препятствие движется влево.
  moveRemainder += speedQ();
  obstacleX -= moveRemainder / 256;
  moveRemainder %= 256;

  // Позиция героя.
  int16_t playerY = L2_GROUND - L2_HERO_H - heightQ / 4;
  h.x = L2_PLAYER_X;
  h.y = playerY;

  // Коллизия (инсеты для прощения антенн и обломков).
  int16_t playerTop = playerY + (sit ? 3 : 1);
  int16_t obY = obstacleY(hazard);
  int16_t obstacleTop = obY + 1;

  bool overlapX = (L2_PLAYER_X + 13 > obstacleX + 2) &&
                  (L2_PLAYER_X + 3 < obstacleX + 14);
  bool overlapY = (playerY + 16 > obstacleTop) &&
                  (playerTop < obY + 15);

  if (overlapX && overlapY && invuln2 == 0) {
    if (h.lives <= 1) {
      h.lives = 0;
      state2 = L2_LOST;
      return;
    }
    --h.lives;
    invuln2 = 60;
  }

  // Очко за пройденное препятствие.
  if (!scored && obstacleX + 16 < L2_PLAYER_X) {
    scored = 1;
    ++h.score;
    if (h.score >= L2_TARGET) {
      state2 = L2_WON;
      return;
    }
  }

  // Респавн препятствия.
  if (obstacleX < -16) {
    obstacleX = 140 + random(0, 25);
    hazard = (uint8_t)random(0, 3);
    scored = 0;
  }
}


void level2Draw(hero &h) {
  // Звёзды.
  for (uint8_t i = 0; i < 9; ++i) {
    uint8_t sx = (i * 29 + 128 - (runFrame / 3) % 128) % 128;
    arduboy.drawPixel(sx, 12 + (i * 7) % 21);
  }

  // Земля и её текстура.
  arduboy.drawFastHLine(0, L2_GROUND, 128);
  for (uint8_t i = 0; i < 8; ++i) {
    uint8_t sx = (i * 19 + 128 - runFrame % 128) % 128;
    arduboy.drawPixel(sx, 60);
  }

  // Препятствие.
  arduboy.drawBitmap(obstacleX, obstacleY(hazard), hazardSprite(hazard), 16, 16, WHITE);

  // Герой — мигает при неуязвимости.
  bool blink = (invuln2 > 0) && ((time_buf % 8) >= 4);
  if (!blink) {
    const uint8_t *robot;
    if (h.status & 1) {
      robot = man_3;
    }
    else if (heightQ > 0 || (runFrame / 6) % 2 == 0) {
      robot = man_1;
    }
    else {
      robot = man_2;
    }
    arduboy.drawBitmap(h.x, h.y, robot, 16, 16, WHITE);
  }

  // HUD.
  arduboy.setCursor(0, 0);
  arduboy.print(F("S:"));
  arduboy.print(h.score);
  arduboy.print(F("/"));
  arduboy.print(L2_TARGET);

  arduboy.setCursor(96, 0);
  arduboy.print(F("L:"));
  arduboy.print(h.lives);

  // Сообщения.
  if (state2 == L2_LOST) {
    arduboy.fillRect(20, 24, 88, 14, BLACK);
    arduboy.drawRect(20, 24, 88, 14, WHITE);
    arduboy.setCursor(36, 28);
    arduboy.print(F("GAME OVER"));
  }
  else if (state2 == L2_WON) {
    arduboy.fillRect(20, 24, 88, 14, BLACK);
    arduboy.drawRect(20, 24, 88, 14, WHITE);
    arduboy.setCursor(30, 28);
    arduboy.print(F("YOU WIN!"));
  }
}


uint8_t level2State() {
  return state2;
}
