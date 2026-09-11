#include <Arduboy2.h>
#include "artemide.h"
#include "obstacles.h"
#include "cat.h"

Arduboy2 arduboy;
constexpr int16_t GROUND = 56;
constexpr int16_t PLAYER_X = 20;
enum GameState : uint8_t { TITLE, RUNNING, CRASHED, TRAVEL };
enum Hazard : uint8_t { BUG, CLOUD, ROCKET };
GameState gameState = TITLE;
Hazard hazard = BUG;
int16_t obstacleX = 140;
int16_t heightQ = 0;  // Quarter-pixel jump height above the ground.
int16_t velocityQ = 0;
uint16_t score = 0;
uint16_t best = 0;    // Best score for this power-on session.
uint16_t runFrame = 0;
uint16_t level = 1;
uint8_t levelPoints = 0;
uint16_t travelFrame = 0;
bool sitting = false;
bool scored = false;

// Gradually approach 6 pixels/frame, keeping later levels playable.
uint16_t speedQ() {
  return 1536 - 1024 * 8UL / (level + 7UL);
}
uint16_t movementRemainder = 0;

void beginTravel() {
  gameState = TRAVEL;
  travelFrame = level == 1 ? 60 : 0;
  heightQ = velocityQ = 0;
  sitting = false;
}

void landOnLevel() {
  gameState = RUNNING;
  obstacleX = 160;
  hazard = BUG;
  levelPoints = 0;
  scored = false;
  movementRemainder = 0;
}

void drawRocket(int16_t x, int16_t y, bool flame, bool catShip) {
  arduboy.fillTriangle(x, y, x - 8, y + 10, x + 8, y + 10, WHITE);
  arduboy.drawRect(x - 8, y + 10, 17, 21, WHITE);
  arduboy.drawCircle(x, y + 16, 3, WHITE);
  // Cat ship has ears on its nose; robot ship has a square hatch.
  if (catShip) {
    arduboy.drawLine(x - 5, y + 5, x - 5, y, WHITE);
    arduboy.drawLine(x + 5, y + 5, x + 5, y, WHITE);
  }
  arduboy.drawRect(x - 3, y + 23, 7, 8, WHITE);
  arduboy.fillTriangle(x - 8, y + 23, x - 13, y + 32, x - 8, y + 31, WHITE);
  arduboy.fillTriangle(x + 8, y + 23, x + 13, y + 32, x + 8, y + 31, WHITE);
  if (flame) {
    arduboy.fillTriangle(x - 5, y + 32, x + 5, y + 32,
                        x, y + 36 + (travelFrame % 4), WHITE);
  }
}

void drawTravel() {
  const uint16_t t = travelFrame;
  arduboy.setCursor(0, 0);
  if (t < 30) arduboy.print(F("THERE'S THE CAT!"));
  else if (t < 60) arduboy.print(F("GOT YOU!"));
  else if (t < 100) arduboy.print(level == 1 ? F("CAT IS ESCAPING!") : F("HEY! COME BACK!"));
  else if (t < 135) arduboy.print(F("NOT AGAIN..."));
  else if (t < 220) arduboy.print(F("AFTER THAT CAT!"));
  else arduboy.print(F("THE CHASE CONTINUES"));
  arduboy.setCursor(0, 9);
  arduboy.print(F("LEVEL "));
  arduboy.print(level);
  arduboy.drawFastHLine(0, GROUND, 128);

  // The cat's separate rocket launches first and never lands with us.
  if (t < 135) {
    const int16_t catShipY = t < 100 ? 23 : 23 - (t - 100) * 3;
    drawRocket(108, catShipY, t >= 100, true);
  }
  int16_t rocketY = 23;
  if (t >= 180 && t < 220) rocketY -= (t - 180) * 3;
  else if (t >= 220 && t < 260) rocketY -= (260 - t) * 3;
  drawRocket(64, rocketY, t >= 180 && t < 260, false);

  // Approach and briefly hold the cat; it slips away to its rocket.
  if (t < 100) {
    int16_t catX = t < 60 ? 58 : 58 + (t - 60);
    arduboy.drawBitmap(catX, GROUND - 16, space_cat, 16, 16, WHITE);
    if (t >= 30 && t < 60) {
      arduboy.drawLine(54, 47, 62, 47, WHITE);
      arduboy.drawLine(54, 50, 62, 50, WHITE);
    }
  }
  if (t < 175 || t >= 260) {
    int16_t robotX = 44;
    if (t < 30) robotX = PLAYER_X + t * 24 / 30;
    else if (t >= 135 && t < 175) robotX = 44 + (t - 135) * 12 / 40;
    else if (t >= 260) robotX = 56 - (t - 260) * 36 / 45;
    const bool walking = t < 30 || (t >= 135 && t < 175) || t >= 260;
    arduboy.drawBitmap(robotX, GROUND - 16,
      walking && (t / 6) % 2 ? man_1 : man_2, 16, 16, WHITE);
  }
  if (++travelFrame >= 305) landOnLevel();
}

void startRun() {
  level = 1;
  levelPoints = 0;
  beginTravel();
  hazard = BUG;
  obstacleX = 140;
  heightQ = velocityQ = 0;
  score = runFrame = 0;
  sitting = scored = false;
}

int16_t obstacleY() {
  // The cloud's bottom is one pixel above the seated antenna.
  return hazard == CLOUD ? 28 : GROUND - 16;
}

void updateRun() {
  ++runFrame;
  sitting = heightQ == 0 &&
    (arduboy.pressed(A_BUTTON) || arduboy.pressed(DOWN_BUTTON));
  if (heightQ == 0 && !sitting &&
      (arduboy.justPressed(B_BUTTON) || arduboy.justPressed(UP_BUTTON))) {
    velocityQ = 17;
  }
  if (velocityQ != 0 || heightQ > 0) {
    heightQ += velocityQ;
    --velocityQ;
    if (heightQ <= 0) {
      heightQ = velocityQ = 0;
    }
  }

  movementRemainder += speedQ();
  obstacleX -= movementRemainder / 256;
  movementRemainder %= 256;
  const int16_t playerY = GROUND - 16 - heightQ / 4;
  // Slightly inset body bounds are forgiving around antennas and debris.
  const int16_t playerTop = playerY + (sitting ? 3 : 1);
  const int16_t obstacleTop = obstacleY() + 1;
  const bool overlapX = PLAYER_X + 13 > obstacleX + 2 &&
                        PLAYER_X + 3 < obstacleX + 14;
  const bool overlapY = playerY + 16 > obstacleTop &&
                        playerTop < obstacleY() + 15;
  if (overlapX && overlapY) {
    gameState = CRASHED;
    return;
  }
  if (!scored && obstacleX + 16 < PLAYER_X) {
    scored = true;
    ++score;
    if (score > best) best = score;
    if (++levelPoints == 25) {
      ++level;
      beginTravel();
      return;
    }
  }
  if (obstacleX < -16) {
    obstacleX = 140 + random(0, 25);
    // Introduce each obstacle in order, then mix them.
    hazard = score < 3 ? static_cast<Hazard>(score % 3) :
                        static_cast<Hazard>(random(0, 3));
    scored = false;
  }
}

void drawRun() {
  arduboy.setCursor(0, 0);
  arduboy.print(F("L"));
  arduboy.print(level);
  arduboy.print(F(" "));
  arduboy.print(levelPoints);
  arduboy.print(F("/25"));
  arduboy.setCursor(96, 0);
  arduboy.print(hazard == CLOUD ? F("DUCK") : F("JUMP"));
  // Sparse scrolling stars and a lunar surface.
  for (uint8_t i = 0; i < 9; ++i) {
    const uint8_t x = (i * 29 + 128 - (runFrame / 3) % 128) % 128;
    arduboy.drawPixel(x, 12 + (i * 7) % 21);
  }
  arduboy.drawFastHLine(0, GROUND, 128);
  for (uint8_t i = 0; i < 8; ++i) {
    arduboy.drawPixel((i * 19 + 128 - runFrame % 128) % 128, 60);
  }
  const uint8_t *sprite = hazard == BUG ? bug_alien :
                          hazard == CLOUD ? dead_pixel_cloud : broken_rocket;
  arduboy.drawBitmap(obstacleX, obstacleY(), sprite, 16, 16, WHITE);
  const uint8_t *robot = sitting ? man_sit :
    (heightQ > 0 || (runFrame / 6) % 2 == 0 ? man_1 : man_2);
  arduboy.drawBitmap(PLAYER_X, GROUND - 16 - heightQ / 4, robot, 16, 16, WHITE);
}

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(30);
  arduboy.initRandomSeed();
}

void loop() {
  if (!arduboy.nextFrame()) return;
  arduboy.pollButtons();
  arduboy.clear();
  if (gameState == TITLE) {
    arduboy.setCursor(25, 2);
    arduboy.print(F("COSMIC CAT"));
    arduboy.setCursor(0, 16);
    arduboy.print(F("UP/B: jump\nDOWN/A: sit\nChase the space cat\n25 points per level"));
    arduboy.setCursor(16, 56);
    arduboy.print(F("B: START MISSION"));
    if (arduboy.justPressed(B_BUTTON)) startRun();
  } else {
    if (gameState == RUNNING) updateRun();
    if (gameState == TRAVEL) drawTravel();
    else drawRun();
    if (gameState == CRASHED) {
      arduboy.fillRect(8, 15, 112, 33, BLACK);
      arduboy.drawRect(8, 15, 112, 33, WHITE);
      arduboy.setCursor(28, 19);
      arduboy.print(F("ROBOT DOWN!"));
      arduboy.setCursor(20, 29);
      arduboy.print(F("BEST "));
      arduboy.print(best);
      arduboy.setCursor(20, 39);
      arduboy.print(F("B: TRY AGAIN"));
      if (arduboy.justPressed(B_BUTTON)) startRun();
    }
  }
  arduboy.display();
}
