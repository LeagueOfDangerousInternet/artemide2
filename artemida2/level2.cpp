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
static uint16_t worldScroll2;

// UFO
static int16_t ufoX;
static int16_t ufoY;
static int8_t ufoVx;
static uint8_t ufoShootTimer;

// Bullet
static uint8_t bulletActive;
static int16_t bulletX;
static int16_t bulletY;
static int16_t bulletVx;
static int16_t bulletVy;


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


// Земляные препятствия — 2 жизни, воздушные — 4.
static uint8_t hazardDamage(uint8_t hz) {
  if (hz == L2_CLOUD) {
    return 4;
  }
  return 2;
}


// Как на первом уровне: правый — разгон, левый — торможение, иначе — сам возвращается к 0.
static void updateRunSpeed(hero &h) {
  if (h.status & 4) {
    h.vx += 1;
    if (h.vx > 3) {
      h.vx = 3;
    }
  }
  else if (h.status & 8) {
    h.vx -= 1;
    if (h.vx < -3) {
      h.vx = -3;
    }
  }
  else {
    if (h.vx > 0) {
      h.vx -= 1;
    }
    else if (h.vx < 0) {
      h.vx += 1;
    }
  }
}


// Итоговая скорость мира: базовая 2 px/frame ± управление (0..5).
static uint16_t speedQ(hero &h) {
  int16_t total = 2 + h.vx;
  if (total < 0) {
    total = 0;
  }
  return (uint16_t)total * 256;
}


static void drawUfo(int16_t x, int16_t y) {
  // Купол
  arduboy.drawCircle(x + 8, y + 5, 3, WHITE);
  // Корпус
  arduboy.drawFastHLine(x + 1, y + 8, 15, WHITE);
  arduboy.drawFastHLine(x, y + 9, 17, WHITE);
  arduboy.drawFastHLine(x + 1, y + 10, 15, WHITE);
  // Огни
  arduboy.drawPixel(x + 3, y + 11, WHITE);
  arduboy.drawPixel(x + 8, y + 11, WHITE);
  arduboy.drawPixel(x + 13, y + 11, WHITE);
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
  worldScroll2 = 0;

  ufoX = 56;
  ufoY = 6;
  ufoVx = 1;
  ufoShootTimer = 0;

  bulletActive = 0;
  bulletX = 0;
  bulletY = 0;
  bulletVx = 0;
  bulletVy = 0;

  h.x = L2_PLAYER_X;
  h.y = L2_GROUND - L2_HERO_H;
  h.vx = 0;
  h.vy = 0;
  h.status = 0;
  h.score = 0;
  h.lives = 12;
}


void level2Update(hero &h) {
  if (state2 == L2_WON || state2 == L2_LOST) {
    return;
  }

  ++runFrame;
  if (invuln2 > 0) {
    invuln2--;
  }

  // Управление скоростью мира: правый — быстрее, левый — медленнее.
  updateRunSpeed(h);

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

  // Мир движется влево со скоростью, зависящей от управления.
  uint16_t frameMoveQ = speedQ(h);
  moveRemainder += frameMoveQ;
  int16_t frameMove = (int16_t)(moveRemainder / 256);
  moveRemainder %= 256;

  obstacleX -= frameMove;
  worldScroll2 += (uint16_t)frameMove;

  // Позиция героя.
  int16_t playerY = L2_GROUND - L2_HERO_H - heightQ / 4;
  h.x = L2_PLAYER_X;
  h.y = playerY;

  // --- UFO ---
  ufoX += ufoVx;
  if (ufoX < 4) {
    ufoX = 4;
    ufoVx = -ufoVx;
  }
  if (ufoX > 108) {
    ufoX = 108;
    ufoVx = -ufoVx;
  }

  // Стрельба раз в ~50 кадров, если пули ещё нет.
  ufoShootTimer++;
  if (ufoShootTimer >= 50 && !bulletActive) {
    ufoShootTimer = 0;
    bulletActive = 1;
    bulletX = ufoX + 8;
    bulletY = ufoY + 13;

    int16_t dx = (L2_PLAYER_X + 8) - bulletX;
    if (dx > 4) {
      bulletVx = 1;
    }
    else if (dx < -4) {
      bulletVx = -1;
    }
    else {
      bulletVx = 0;
    }
    bulletVy = 2;
  }

  // Полёт пули.
  if (bulletActive) {
    bulletX += bulletVx;
    bulletY += bulletVy;
    if (bulletY > 64 || bulletX < -4 || bulletX > 132) {
      bulletActive = 0;
    }
  }

  // --- Коллизия: герой vs земляное/воздушное препятствие ---
  int16_t playerTop = playerY + (sit ? 3 : 1);
  int16_t obY = obstacleY(hazard);
  int16_t obstacleTop = obY + 1;

  bool overlapX = (L2_PLAYER_X + 13 > obstacleX + 2) &&
                  (L2_PLAYER_X + 3 < obstacleX + 14);
  bool overlapY = (playerY + 16 > obstacleTop) &&
                  (playerTop < obY + 15);

  if (overlapX && overlapY && invuln2 == 0) {
    uint8_t damage = hazardDamage(hazard);
    if (h.lives <= damage) {
      h.lives = 0;
      state2 = L2_LOST;
      return;
    }
    h.lives -= damage;
    invuln2 = 60;
  }

  // --- Коллизия: герой vs UFO (4 жизни) ---
  if (invuln2 == 0) {
    int16_t ufoLeft = ufoX + 1;
    int16_t ufoRight = ufoX + 16;
    int16_t ufoTop = ufoY + 2;
    int16_t ufoBottom = ufoY + 12;

    bool ufoOverlapX = (L2_PLAYER_X + 13 > ufoLeft) &&
                       (L2_PLAYER_X + 3 < ufoRight);
    bool ufoOverlapY = (playerY + 16 > ufoTop) &&
                       (playerTop < ufoBottom);

    if (ufoOverlapX && ufoOverlapY) {
      if (h.lives <= 4) {
        h.lives = 0;
        state2 = L2_LOST;
        return;
      }
      h.lives -= 4;
      invuln2 = 60;
    }
  }

  // --- Коллизия: герой vs пуля (1 жизнь) ---
  if (invuln2 == 0 && bulletActive) {
    bool bOverlapX = (L2_PLAYER_X + 13 > bulletX) &&
                     (L2_PLAYER_X + 3 < bulletX + 3);
    bool bOverlapY = (playerY + 16 > bulletY + 1) &&
                     (playerTop < bulletY + 3);

    if (bOverlapX && bOverlapY) {
      bulletActive = 0;
      if (h.lives <= 1) {
        h.lives = 0;
        state2 = L2_LOST;
        return;
      }
      h.lives -= 1;
      invuln2 = 30;
    }
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
  // Звёзды — с параллаксом, чтобы чувствовалась скорость.
  for (uint8_t i = 0; i < 9; ++i) {
    int16_t sx = (int16_t)(i * 29 + 128 - ((worldScroll2 / 3) % 128)) % 128;
    if (sx < 0) {
      sx += 128;
    }
    arduboy.drawPixel(sx, 12 + (i * 7) % 21);
  }

  // Земля и её текстура.
  arduboy.drawFastHLine(0, L2_GROUND, 128);
  for (uint8_t i = 0; i < 8; ++i) {
    int16_t sx = (int16_t)(i * 19 + 128 - (worldScroll2 % 128)) % 128;
    if (sx < 0) {
      sx += 128;
    }
    arduboy.drawPixel(sx, 60);
  }

  // Препятствие.
  arduboy.drawBitmap(obstacleX, obstacleY(hazard), hazardSprite(hazard), 16, 16, WHITE);

  // UFO.
  drawUfo(ufoX, ufoY);

  // Пуля.
  if (bulletActive) {
    arduboy.fillCircle(bulletX + 1, bulletY + 1, 1, WHITE);
  }

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

  // 3 сердца, каждое = 4 жизни (2 полу-сердца × 2 жизни).
  for (int slot = 0; slot < 3; slot++) {
    int hx = 104 + slot * 8;
    uint8_t full = (uint8_t)((slot + 1) * 4);
    uint8_t half = (uint8_t)(full - 2);
    const uint8_t *sprite = heart_3;
    if (h.lives >= full) {
      sprite = heart_1;
    }
    else if (h.lives >= half) {
      sprite = heart_2;
    }
    arduboy.drawBitmap(hx, 0, sprite, 8, 8, WHITE);
  }

  // Сообщения.
  if (state2 == L2_LOST) {
    arduboy.fillRect(20, 24, 88, 14, BLACK);
    arduboy.drawRect(20, 24, 88, 14, WHITE);
    arduboy.setCursor(37, 28);
    arduboy.print(F("GAME OVER"));
  }
  else if (state2 == L2_WON) {
    arduboy.fillRect(20, 24, 88, 14, BLACK);
    arduboy.drawRect(20, 24, 88, 14, WHITE);
    arduboy.setCursor(40, 28);
    arduboy.print(F("YOU WIN!"));
  }
}


uint8_t level2State() {
  return state2;
}
