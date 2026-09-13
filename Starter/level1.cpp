#include "artemide.h"
#include "level1.h"


static uint8_t groundTop[STRIP_TILES];

// Один байт на тайл:
//   биты 0..2 — флаги тайла (TF_OBSTACLE / TF_WIRE / TF_CEILING)
//   биты 3..4 — монета (COIN_NONE / COIN_PLUS / COIN_NILE)
static uint8_t tileData[STRIP_TILES];

#define TF_OBSTACLE 0x01
#define TF_WIRE     0x02
#define TF_CEILING  0x04

#define COIN_SHIFT  3
#define COIN_MASK   0x18
#define COIN_NONE   0
#define COIN_PLUS   1
#define COIN_NILE   2

static int16_t worldX;
static int16_t camX;
static uint8_t state;
static uint8_t runSpeed;

// состояние генератора
static int16_t frontier;
static uint8_t curH;
static uint8_t segLeft;
static uint8_t segLen;
static uint8_t segPos;
static uint8_t segObsAt;
static uint8_t segWireAt;
static uint8_t pitLeft;
static uint8_t coinCooldown;

// физика/состояние
static uint8_t invuln;
static uint8_t jumpBuffer;


// Провод висит на 3 блока (24 px) над землёй, спрайт свисает вниз.
#define WIRE_TOP_OFFSET 24
#define WIRE_BOT_OFFSET 13
#define CEIL_OFFSET 32

// Хитбоксы
#define HERO_HIT_H 13   // визуально занято ~13 строк из 16
#define HERO_HIT_X 3    // по 3 px пустых с боков спрайта
#define COMP_HIT_TOP 6    // комп визуально 8 px, для хитбокса 6


// Хелперы доступа к tileData.
static uint8_t tileFlags(uint8_t idx) {
  return tileData[idx] & 0x07;
}

static uint8_t tileCoin(uint8_t idx) {
  return (tileData[idx] & COIN_MASK) >> COIN_SHIFT;
}

static void setTileCoin(uint8_t idx, uint8_t value) {
  tileData[idx] = (tileData[idx] & ~COIN_MASK) | ((value << COIN_SHIFT) & COIN_MASK);
}


static void generateTile() {
  uint8_t idx = (uint8_t)(frontier % STRIP_TILES);

  // ЯМА
  if (pitLeft > 0) {
    groundTop[idx] = PIT;
    tileData[idx] = 0;
    pitLeft--;
    if (coinCooldown > 0) {
      coinCooldown--;
    }
    frontier++;
    return;
  }

  // НОВЫЙ СЕГМЕНТ
  if (segLeft == 0) {
    uint8_t len = 8 + random(0, 5);

    if (frontier < 10) {
      len = 14;
      curH = 48;
    }
    else {
      int8_t delta = (int8_t)(random(0, 3)) - 1;
      int newH = (int)curH + delta * 8;
      if (newH < 40) {
        newH = 40;
      }
      if (newH > 56) {
        newH = 56;
      }
      curH = newH;
    }

    segLen = len;
    segLeft = len;
    segPos = 0;

    segObsAt = 255;
    segWireAt = 255;

    if (frontier >= 10 && len >= 7) {
      int eventRoll = random(0, 100);
      if (eventRoll < 7) {
        segObsAt = 2 + random(0, len - 4);
      }
      else if (eventRoll < 15) {
        segWireAt = 2 + random(0, len - 5);
      }
    }
  }

  // ЗЕМЛЯ
  groundTop[idx] = curH;

  // ФЛАГИ ТАЙЛА
  uint8_t f = 0;
  if (segPos == segObsAt) {
    f |= TF_OBSTACLE;
  }
  if (segPos == segWireAt) {
    f |= TF_WIRE;
  }
  if (segWireAt != 255 && (segPos == segWireAt || segPos == segWireAt + 1)) {
    f |= TF_CEILING;
  }
  tileData[idx] = f;

  // МОНЕТА
  if (f == 0 && coinCooldown == 0) {
    bool nearObs = false;
    if (segObsAt != 255) {
      uint8_t d;
      if (segPos >= segObsAt) {
        d = segPos - segObsAt;
      }
      else {
        d = segObsAt - segPos;
      }
      if (d <= 2) {
        nearObs = true;
      }
    }

    bool nearWire = false;
    if (segWireAt != 255) {
      if (segPos + 1 >= segWireAt && segPos <= segWireAt + 2) {
        nearWire = true;
      }
    }

    bool edgeTile = (segPos < 2) || (segPos >= segLen - 1);

    if (!nearObs && !nearWire && !edgeTile) {
      int roll = random(0, 100);
      if (roll < 45) {
        setTileCoin(idx, COIN_PLUS);
        coinCooldown = 3;
      }
      else if (roll < 50) {
        setTileCoin(idx, COIN_NILE);
        coinCooldown = 3;
      }
    }
  }

  if (coinCooldown > 0) {
    coinCooldown--;
  }

  segPos++;
  segLeft--;
  frontier++;

  if ((segLeft == 0) && (frontier > 16) && (random(0, 100) < 15)) {
    pitLeft = 2 + random(0, 2);
  }
}


static void updateHeroVx(hero &h) {
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


void level1Init(hero &h) {
  randomSeed(millis());

  frontier = 0;
  curH = 48;
  segLeft = 0;
  segLen = 0;
  segPos = 0;
  segObsAt = 255;
  segWireAt = 255;
  pitLeft = 0;
  coinCooldown = 0;

  while (frontier < STRIP_TILES) {
    generateTile();
  }

  worldX = 16;
  camX = 0;
  state = 0;
  runSpeed = 2;
  invuln = 0;
  jumpBuffer = 0;

  h.x = HERO_SCREEN_X;
  h.y = 48 - HERO_H;
  h.vx = 0;
  h.vy = 0;
  h.score = 0;
  h.status = 0;
  h.lives = 6;
}

void level1Update(hero &h) {
  if (state == 2 || state == 1) {
    return;
  }

  int16_t needUntil = (worldX / TILE_SIZE) + 30 - 4;
  while (frontier < needUntil) {
    generateTile();
  }

  if (invuln > 0) {
    invuln--;
  }

  updateHeroVx(h);

  int16_t totalVx = (int16_t)runSpeed + h.vx;
  if (totalVx < 0) {
    totalVx = 0;
  }
  worldX += totalVx;

  camX = worldX - HERO_SCREEN_X;
  if (camX < 0) {
    camX = 0;
  }
  h.x = worldX - camX;

  // тайл под центром героя
  int16_t feetX = worldX + HERO_W / 2;
  int feetTileSigned = feetX / TILE_SIZE;
  int ftM = feetTileSigned % STRIP_TILES;
  if (ftM < 0) {
    ftM += STRIP_TILES;
  }
  uint8_t feetTile = (uint8_t)ftM;

  uint8_t gtop = groundTop[feetTile];
  bool hasCeiling = (tileFlags(feetTile) & TF_CEILING) != 0;
  int16_t ceilY = (int16_t)gtop - CEIL_OFFSET;

  // onGround — узкий диапазон, только «стою ровно»
  bool onGround = false;
  if (gtop != PIT) {
    int16_t bottom = h.y + HERO_H;
    if (hasCeiling && bottom >= ceilY && bottom <= ceilY + 1) {
      onGround = true;
    }
    else if (bottom >= gtop && bottom <= gtop + 1) {
      onGround = true;
    }
  }

  // буфер прыжка
  if (status & 2) {
    jumpBuffer = 8;
  }
  if (jumpBuffer > 0 && onGround) {
    h.vy = -5;
    jumpBuffer = 0;
  }
  if (jumpBuffer > 0) {
    jumpBuffer--;
  }

  // движение по вертикали
  h.y += h.vy;

  // приземление — только если герой ПАДАЕТ (vy > 0)
  // и его ноги на границе или чуть ниже блока.
  // Если герой «въехал» в блок сбоку без прыжка — vy = 0 или
  // bottom уже ниже gtop + TILE_SIZE, приземление не сработает
  // и он провалится в тайл.
  if (gtop != PIT) {
    int16_t bottom = h.y + HERO_H;
    bool landed = false;

    if (hasCeiling && h.vy > 0 && bottom > ceilY && bottom <= ceilY + TILE_SIZE) {
      h.y = ceilY - HERO_H;
      h.vy = 0;
      landed = true;
    }

    if (!landed && h.vy > 0 && bottom > gtop && bottom <= gtop + TILE_SIZE) {
      h.y = gtop - HERO_H;
      h.vy = 0;
    }
  }

  // гравитация ПОСЛЕ приземления, чтобы одного кадра
  // недостаточно было проскочить сквозь ступеньку
  if (time % 2 == 0) {
    h.vy += 1;
    if (h.vy > 5) {
      h.vy = 5;
    }
  }

  if (h.y > FALL_Y) {
    state = 2;
    return;
  }

  // хитбокс головы: присед опускает на 3 px
  int16_t headY = h.y;
  if (h.status & 1) {
    headY = h.y + 3;
  }

  // границы по X — сужены для хитбокса
  int16_t hLeft  = worldX + HERO_HIT_X;
  int16_t hRight = worldX + HERO_W - HERO_HIT_X;
  int16_t hBottom = h.y + HERO_HIT_H;

  int idxL = (hLeft) / TILE_SIZE - 1;
  int idxR = (hRight) / TILE_SIZE + 1;

  for (int i = idxL; i <= idxR; i++) {
    if (i < 0) {
      continue;
    }
    int m = i % STRIP_TILES;
    if (m < 0) {
      m += STRIP_TILES;
    }
    uint8_t idx = (uint8_t)m;

    uint8_t tf = tileFlags(idx);

    // ПРЕПЯТСТВИЕ — 2 жизни
    if ((tf & TF_OBSTACLE) && invuln == 0) {
      uint8_t gt = groundTop[idx];
      if (gt != PIT) {
        int16_t otop = (int16_t)gt - COMP_HIT_TOP;
        int16_t compL = i * TILE_SIZE + 1;
        int16_t compR = i * TILE_SIZE + 7;

        bool overlapX = (hRight > compL) && (hLeft < compR);
        bool overlapY = (h.y < gt) && (hBottom > otop);

        if (overlapX && overlapY) {
          if (h.lives <= 2) {
            h.lives = 0;
            state = 2;
            return;
          }
          h.lives -= 2;
          invuln = 60;
        }
      }
    }

    // ПРОВОД — 1 жизнь
    if ((tf & TF_WIRE) && invuln == 0) {
      uint8_t gt = groundTop[idx];
      if (gt != PIT) {
        int16_t wireX = i * TILE_SIZE;
        int16_t wireR = wireX + 16;
        int16_t wireT = (int16_t)gt - WIRE_TOP_OFFSET;
        int16_t wireB = (int16_t)gt - WIRE_BOT_OFFSET;

        bool overlapX = (hRight > wireX) && (hLeft < wireR);
        bool overlapY = (hBottom > wireT) && (headY < wireB);

        if (overlapX && overlapY) {
          if (h.lives <= 1) {
            h.lives = 0;
            state = 2;
            return;
          }
          h.lives -= 1;
          invuln = 60;
        }
      }
    }

    // МОНЕТА
    uint8_t cn = tileCoin(idx);
    if (cn != COIN_NONE) {
      uint8_t gt = groundTop[idx];
      if (gt != PIT) {
        int16_t coinX = i * TILE_SIZE;
        int16_t coinY = gt - 8;

        bool overlapX = (hRight > coinX) && (hLeft < coinX + 8);
        bool overlapY = (hBottom > coinY) && (h.y < coinY + 8);

        if (overlapX && overlapY) {
          if (cn == COIN_PLUS) {
            h.score += 1;
          }
          else if (cn == COIN_NILE) {
            if (h.score > 0) {
              h.score -= 1;
            }
          }
          setTileCoin(idx, COIN_NONE);
        }
      }
    }
  }

  if (h.score >= 64) {
    state = 1;
  }
}


void level1Draw(hero &h) {
  int firstTile = camX / TILE_SIZE - 2;
  if (firstTile < 0) {
    firstTile = 0;
  }
  int lastTile = (camX + 128) / TILE_SIZE + 1;

  for (int i = firstTile; i < lastTile; i++) {
    int m = i % STRIP_TILES;
    if (m < 0) {
      m += STRIP_TILES;
    }
    uint8_t idx = (uint8_t)m;

    int sx = i * TILE_SIZE - camX;
    uint8_t gt = groundTop[idx];
    uint8_t tf = tileFlags(idx);

    if (gt != PIT) {
      arduboy.drawBitmap(sx, gt, block, 8, 8, WHITE);
    }

    if ((tf & TF_OBSTACLE) && gt != PIT) {
      arduboy.drawBitmap(sx, gt - 8, comp, 8, 8, WHITE);
    }

    // Потолок над проводом — block2. Рисуется ДО провода.
    if ((tf & TF_CEILING) && gt != PIT) {
      arduboy.drawBitmap(sx, (int16_t)gt - CEIL_OFFSET, block2, 8, 8, WHITE);
    }

    if ((tf & TF_WIRE) && gt != PIT) {
      arduboy.drawBitmap(sx, (int16_t)gt - WIRE_TOP_OFFSET, wire, 16, 16, WHITE);
    }

    uint8_t cn = tileCoin(idx);
    if (cn != COIN_NONE && gt != PIT) {
      if (cn == COIN_PLUS) {
        if (time_buf < 15) {
          arduboy.drawBitmap(sx, gt - 8, uno_1, 8, 8, WHITE);
        }
        else if (time_buf < 30 || time_buf > 45) {
          arduboy.drawBitmap(sx, gt - 8, uno_2, 8, 8, WHITE);
        }
        else {
          arduboy.drawBitmap(sx, gt - 8, uno_3, 8, 8, WHITE);
        }
      }
      else if (cn == COIN_NILE) {
        if ((time_buf / 2) % 2 < 1) {
          arduboy.drawBitmap(sx, gt - 8, nile_1, 8, 8, WHITE);
        }
        else {
          arduboy.drawBitmap(sx, gt - 8, uno_2, 8, 8, WHITE);
        }
      }
    }
  }

  if (invuln == 0 || (time_buf % 8) < 4) {
    h.draw();
  }

  arduboy.setCursor(0, 0);
  arduboy.print(F("S:"));
  arduboy.print(h.score);
  arduboy.print(F("/64"));

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

  if (state == 2) {
    arduboy.fillRect(20, 24, 88, 14, BLACK);
    arduboy.drawRect(20, 24, 88, 14, WHITE);
    arduboy.setCursor(37, 28);
    arduboy.print(F("GAME OVER"));
  }
  else if (state == 1) {
    arduboy.fillRect(20, 24, 88, 14, BLACK);
    arduboy.drawRect(20, 24, 88, 14, WHITE);
    arduboy.setCursor(40, 28);
    arduboy.print(F("YOU WIN!"));
  }
}



uint8_t level1State() {
  return state;
}
