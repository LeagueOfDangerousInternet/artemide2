#include <Arduboy2.h>
#include "artemide.h"
#include "obstacles.h"

Arduboy2 arduboy;
constexpr int16_t GROUND = 56;
constexpr int16_t PLAYER_X = 20;
enum GameState : uint8_t { TITLE, RUNNING, CRASHED };
enum Hazard : uint8_t { BUG, CLOUD, ROCKET };
GameState gameState = TITLE;
Hazard hazard = BUG;
int16_t obstacleX = 140;
int16_t heightQ = 0;  // Quarter-pixel jump height above the ground.
int16_t velocityQ = 0;
uint16_t score = 0;
uint16_t best = 0;    // Best score for this power-on session.
uint16_t runFrame = 0;
bool sitting = false;
bool scored = false;

void startRun() {
  gameState = RUNNING;
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

  obstacleX -= 2;
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
  arduboy.print(F("SCORE "));
  arduboy.print(score);
  arduboy.setCursor(78, 0);
  arduboy.print(hazard == CLOUD ? F("DUCK!") : F("JUMP!"));
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
    arduboy.print(F("SPACE DEBUG"));
    arduboy.setCursor(0, 16);
    arduboy.print(F("UP/B: jump\nDOWN/A: sit\nJump bugs + rockets\nDuck pixel clouds"));
    arduboy.setCursor(16, 56);
    arduboy.print(F("B: START MISSION"));
    if (arduboy.justPressed(B_BUTTON)) startRun();
  } else {
    if (gameState == RUNNING) updateRun();
    drawRun();
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
