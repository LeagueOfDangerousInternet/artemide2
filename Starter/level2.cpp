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
// IMPROVEMENT: rare moving gaps make the lunar ground irregular and allow
// falling without changing the submitted jump or obstacle controls.
static int16_t pitX;
static bool falling2;
static int16_t fallOffset2;

// IMPROVEMENT: mirror level 1's horizontal acceleration. Right raises the
// scrolling speed from 2 to 5 px/frame; Left can slow it to zero.
static void updateRunSpeed(hero &h) {
  if (h.status & 4) {
    if (h.vx < 3) ++h.vx;
  }
  else if (h.status & 8) {
    if (h.vx > -2) --h.vx;
  }
  else if (h.vx > 0) {
    --h.vx;
  }
  else if (h.vx < 0) {
    ++h.vx;
  }
}


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


static uint16_t speedQ(const hero &h) {
  // 6 пикселей за кадр в 1/256-х долях пикселя.
  // IMPROVEMENT: use level 1's 2..5 px/frame input-controlled speed range.
  return (uint16_t)(2 + h.vx) * 256;
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
  // IMPROVEMENT: begin with a long safe stretch before the first rare pit.
  pitX = 360;
  falling2 = false;
  fallOffset2 = 0;

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

  // IMPROVEMENT: apply the submitted directional status bits to run speed.
  updateRunSpeed(h);

  // IMPROVEMENT: move the rare terrain gap at exactly the obstacle/map speed.
  uint16_t frameMoveQ = speedQ(h);
  int16_t frameMove = (int16_t)((moveRemainder + frameMoveQ) / 256);
  pitX -= frameMove;
  if (pitX < -24) {
    pitX = 300 + random(0, 260);
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
  moveRemainder += frameMoveQ;
  obstacleX -= moveRemainder / 256;
  moveRemainder %= 256;

  // Позиция героя.
  int16_t playerY = L2_GROUND - L2_HERO_H - heightQ / 4;
  // IMPROVEMENT: stepping over a pit while grounded starts a visible fall.
  bool overPit = (L2_PLAYER_X + 11 > pitX) && (L2_PLAYER_X + 5 < pitX + 22);
  if (!falling2 && onGround && overPit) {
    falling2 = true;
  }
  if (falling2) {
    // IMPROVEMENT: accumulate the fall so the robot visibly drops off-screen.
    fallOffset2 += 3;
    playerY += fallOffset2;
    if (playerY > 64) {
      state2 = L2_LOST;
      return;
    }
  }
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
  // IMPROVEMENT: match level 1's 8x8 block-tile map style. Level 2 remains
  // distinct through its flat lunar surface, stars, craters and space hazards.
  uint8_t groundOffset = (uint8_t)((runFrame * (2 + h.vx)) % TILE_SIZE);
  for (int16_t sx = -(int16_t)groundOffset; sx < 128; sx += TILE_SIZE) {
    // IMPROVEMENT: omit tiles intersecting the rare 22-pixel pit.
    if (sx + TILE_SIZE <= pitX || sx >= pitX + 22) {
      arduboy.drawBitmap(sx, L2_GROUND, block, TILE_SIZE, TILE_SIZE, WHITE);
    }
  }
  for (uint8_t i = 0; i < 5; ++i) {
    uint8_t sx = (i * 31 + 128 - (runFrame * (2 + h.vx)) % 128) % 128;
    arduboy.drawPixel(sx, 59);
  }

  // IMPROVEMENT: acceleration streaks make high speed immediately visible.
  if (h.vx >= 2) {
    for (uint8_t i = 0; i < 4; ++i) {
      arduboy.drawFastHLine((runFrame * 9 + i * 31) % 96,
                            14 + i * 9, 8, WHITE);
    }
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

  // IMPROVEMENT: match level 1's three two-part heart indicators.
  for (int slot = 0; slot < 3; ++slot) {
    int hx = 104 + slot * 8;
    uint8_t need = (uint8_t)(slot * 2);
    const uint8_t *sprite = heart_3;
    if (h.lives >= need + 2) sprite = heart_1;
    else if (h.lives >= need + 1) sprite = heart_2;
    arduboy.drawBitmap(hx, 0, sprite, 8, 8, WHITE);
  }

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
