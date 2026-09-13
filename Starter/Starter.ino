// IMPROVEMENT: the submitted source is preserved in "submitted version".
// This active entry point adds menus, story cutscenes, failure choices and level 3.
#include <Arduboy2.h>
#include "artemide.h"
#include "bkup/cat.h"

Arduboy2 arduboy;
ArduboyTones tones(arduboy.audio.enabled);
hero player;

uint8_t time = 0;
uint8_t time_buf = 0;
uint8_t status = 0;
uint8_t height = 48;
uint8_t currentLevel = 1;

// IMPROVEMENT: scenes wrap the submitted level modules without modifying their
// public state contract (0 running, 1 won, 2 lost).
enum AppScene : uint8_t {
  START_MENU, INTRO_STORY, PLAY_LEVEL, LEVEL1_END, LEVEL2_START,
  LEVEL2_END, PLAY_LEVEL3, FINAL_END, FAIL_MENU
};
AppScene scene = START_MENU;
uint16_t sceneFrame = 0;
uint8_t failChoice = 0;

static void drawCat(int16_t x, int16_t y) {
  arduboy.drawBitmap(x, y, space_cat, 16, 16, WHITE);
}

// IMPROVEMENT: reusable cat-shaped rocket. The ear fins distinguish it from
// the robot rocket even on the 128x64 monochrome screen.
static void drawRocket(int16_t x, int16_t y, bool catRocket, bool flame) {
  arduboy.fillTriangle(x, y, x - 8, y + 10, x + 8, y + 10, WHITE);
  arduboy.drawRect(x - 8, y + 10, 17, 20, WHITE);
  arduboy.drawCircle(x, y + 16, 3, WHITE);
  if (catRocket) {
    arduboy.drawLine(x - 6, y + 6, x - 6, y, WHITE);
    arduboy.drawLine(x + 6, y + 6, x + 6, y, WHITE);
  }
  arduboy.fillTriangle(x - 8, y + 23, x - 13, y + 31, x - 8, y + 29, WHITE);
  arduboy.fillTriangle(x + 8, y + 23, x + 13, y + 31, x + 8, y + 29, WHITE);
  if (flame) arduboy.fillTriangle(x - 4, y + 30, x + 4, y + 30,
                                  x, y + 36 + sceneFrame % 4, WHITE);
}

// IMPROVEMENT: portal animation used for the transition from level 1 to 2.
static void drawPortal(int16_t x, int16_t y) {
  uint8_t pulse = (sceneFrame / 4) % 3;
  arduboy.drawCircle(x, y, 9 + pulse, WHITE);
  arduboy.drawCircle(x, y, 5 + pulse, WHITE);
  arduboy.drawPixel(x, y, WHITE);
}

// IMPROVEMENT: simple vehicles for the level 2 ending and driving finale.
static void drawCar(int16_t x, int16_t y) {
  arduboy.drawRect(x, y + 5, 25, 8, WHITE);
  arduboy.drawRect(x + 5, y, 13, 6, WHITE);
  arduboy.drawCircle(x + 5, y + 14, 3, WHITE);
  arduboy.drawCircle(x + 20, y + 14, 3, WHITE);
}
static void drawBike(int16_t x, int16_t y) {
  arduboy.drawCircle(x, y + 9, 4, WHITE);
  arduboy.drawCircle(x + 14, y + 9, 4, WHITE);
  arduboy.drawLine(x, y + 9, x + 6, y + 2, WHITE);
  arduboy.drawLine(x + 6, y + 2, x + 14, y + 9, WHITE);
  arduboy.drawLine(x, y + 9, x + 10, y + 9, WHITE);
}

static void beginScene(AppScene next) {
  scene = next;
  sceneFrame = 0;
}

static void beginLevel(uint8_t number) {
  currentLevel = number;
  if (number == 1) level1Init(player);
  else if (number == 2) level2Init(player);
  else level3Init(player);
  beginScene(number == 3 ? PLAY_LEVEL3 : PLAY_LEVEL);
}

// IMPROVEMENT: opening story: cat boards and launches, robot reacts and follows,
// then the cat lands and runs across the first level.
static void drawIntro() {
  arduboy.drawFastHLine(0, 56, 128, WHITE);
  if (sceneFrame < 70) {
    arduboy.setCursor(21, 2); arduboy.print(F("A CAT ON MARS?"));
    drawRocket(105, 22, true, false);
    drawCat(10 + sceneFrame, 40);
  }
  else if (sceneFrame < 125) {
    arduboy.setCursor(24, 2); arduboy.print(F("THE CAT ESCAPES!"));
    drawRocket(105, 22 - (sceneFrame - 70) * 2, true, true);
  }
  else if (sceneFrame < 175) {
    arduboy.setCursor(12, 2); arduboy.print(F("ROBOT: !!!"));
    arduboy.drawBitmap(18 + (sceneFrame - 125) / 2, 40,
      (sceneFrame / 5) % 2 ? man_1 : man_4, 16, 16, WHITE);
    drawRocket(91, 22, false, false);
  }
  else if (sceneFrame < 225) {
    arduboy.setCursor(15, 2); arduboy.print(F("START THE CHASE!"));
    drawRocket(91, 22 - (sceneFrame - 175) * 2, false, true);
  }
  else {
    arduboy.setCursor(15, 2); arduboy.print(F("LEVEL 1: LANDED"));
    drawRocket(18, 22, true, false);
    drawCat(20 + (sceneFrame - 225) * 2, 40);
  }
  if (++sceneFrame >= 275) beginLevel(1);
}

// IMPROVEMENT: level 1 ending: reunion, distraction, portal escape and pursuit.
static void drawLevel1End() {
  arduboy.drawFastHLine(0, 56, 128, WHITE);
  arduboy.setCursor(0, 0);
  if (sceneFrame < 55) {
    arduboy.print(F("FOUND YOU!"));
    arduboy.drawBitmap(35, 40, man_1, 16, 16, WHITE);
    drawCat(67, 40);
  } else if (sceneFrame < 100) {
    arduboy.print(F("LOOK! A SPACE BUG!"));
    arduboy.drawBitmap(35, 40, man_4, 16, 16, WHITE);
    drawCat(67 + (sceneFrame - 55), 40);
  } else if (sceneFrame < 145) {
    arduboy.print(F("CAT: BYE!"));
    drawPortal(103, 42);
    drawCat(67 + (sceneFrame - 100), 40);
  } else {
    arduboy.print(F("FOLLOW THAT CAT!"));
    drawPortal(103, 42);
    arduboy.drawBitmap(35 + (sceneFrame - 145) * 2, 40,
      (sceneFrame / 5) % 2 ? man_1 : man_2, 16, 16, WHITE);
  }
  if (++sceneFrame >= 185) beginScene(LEVEL2_START);
}

// IMPROVEMENT: the cat exits the portal and runs left-to-right before level 2.
static void drawLevel2Start() {
  arduboy.drawFastHLine(0, 56, 128, WHITE);
  arduboy.setCursor(13, 0); arduboy.print(F("LEVEL 2: DEEP SPACE"));
  drawPortal(12, 42);
  drawCat(14 + sceneFrame * 2, 40);
  if (++sceneFrame >= 55) beginLevel(2);
}

// IMPROVEMENT: level 2 ending introduces the bicycle and car used by the chase.
static void drawLevel2End() {
  arduboy.drawFastHLine(0, 58, 128, WHITE);
  arduboy.setCursor(0, 0);
  if (sceneFrame < 50) {
    arduboy.print(F("CAUGHT YOU AGAIN!"));
    arduboy.drawBitmap(25, 42, man_1, 16, 16, WHITE);
    drawCat(55, 42); drawBike(82, 44); drawCar(99, 41);
  } else if (sceneFrame < 110) {
    arduboy.print(F("CAT TAKES THE BIKE!"));
    drawCat(55 + sceneFrame - 50, 34);
    drawBike(55 + sceneFrame - 50, 44);
    drawCar(92, 41);
  } else {
    arduboy.print(F("ROBOT TAKES THE CAR!"));
    drawCar(25 + (sceneFrame - 110) * 2, 41);
  }
  if (++sceneFrame >= 160) beginLevel(3);
}

// IMPROVEMENT: final reunion ends the three-level story.
static void drawEnding() {
  arduboy.setCursor(21, 2); arduboy.print(F("CHASE COMPLETE"));
  arduboy.drawFastHLine(0, 56, 128, WHITE);
  drawCar(15, 40);
  drawCat(86, 40);
  arduboy.drawBitmap(61, 40, man_1, 16, 16, WHITE);
  arduboy.setCursor(20, 20); arduboy.print(F("FRIENDS AT LAST!"));
  arduboy.setCursor(19, 58); arduboy.print(F("A: MAIN MENU"));
  if (arduboy.justPressed(A_BUTTON)) beginScene(START_MENU);
}

// IMPROVEMENT: failure menu supports retrying the current level or resetting
// the full story. It replaces the submitted one-action retry prompt.
static void drawFailMenu() {
  arduboy.setCursor(31, 5); arduboy.print(F("MISSION FAILED"));
  if (arduboy.justPressed(UP_BUTTON) || arduboy.justPressed(DOWN_BUTTON))
    failChoice ^= 1;
  arduboy.setCursor(20, 27);
  arduboy.print(failChoice == 0 ? F("> RETRY LEVEL") : F("  RETRY LEVEL"));
  arduboy.setCursor(20, 39);
  arduboy.print(failChoice == 1 ? F("> RESET STORY") : F("  RESET STORY"));
  arduboy.setCursor(13, 55); arduboy.print(F("A/B: SELECT"));
  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
    if (failChoice == 0) beginLevel(currentLevel);
    else beginScene(START_MENU);
  }
}

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(30);
  arduboy.invert(true);
  arduboy.audio.begin();
  tones.tones(interstellar);
}

void loop() {
  if (!arduboy.nextFrame()) return;
  arduboy.pollButtons();
  arduboy.clear();
  ++time;
  time_buf = time % 60;

  // IMPROVEMENT: a real starting menu now gates the story.
  if (scene == START_MENU) {
    arduboy.setCursor(26, 5); arduboy.print(F("COSMIC CAT"));
    arduboy.setCursor(17, 20); arduboy.print(F("A ROBOT ADVENTURE"));
    drawCat(13, 38);
    arduboy.drawBitmap(99, 38, man_1, 16, 16, WHITE);
    arduboy.setCursor(31, 56); arduboy.print(F("A: START"));
    if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON))
      beginScene(INTRO_STORY);
  }
  else if (scene == INTRO_STORY) drawIntro();
  else if (scene == LEVEL1_END) drawLevel1End();
  else if (scene == LEVEL2_START) drawLevel2Start();
  else if (scene == LEVEL2_END) drawLevel2End();
  else if (scene == FINAL_END) drawEnding();
  else if (scene == FAIL_MENU) drawFailMenu();
  else {
    // IMPROVEMENT: preserve the submitted status-bit controls for levels 1/2.
    status = 0;
    if (arduboy.pressed(DOWN_BUTTON)) status |= 1;
    if (arduboy.justPressed(UP_BUTTON)) status |= 2;
    if (arduboy.pressed(RIGHT_BUTTON)) status |= 4;
    if (arduboy.pressed(LEFT_BUTTON)) status |= 8;
    player.update(status);

    if (scene == PLAY_LEVEL3) {
      level3Update(player);
      level3Draw(player);
      uint8_t s = level3State();
      if (s == L3_WON) beginScene(FINAL_END);
      else if (s == L3_LOST) { failChoice = 0; beginScene(FAIL_MENU); }
    } else if (currentLevel == 1) {
      level1Update(player);
      level1Draw(player);
      uint8_t s = level1State();
      if (s == 1) beginScene(LEVEL1_END);
      else if (s == 2) { failChoice = 0; beginScene(FAIL_MENU); }
    } else {
      level2Update(player);
      level2Draw(player);
      uint8_t s = level2State();
      if (s == L2_WON) beginScene(LEVEL2_END);
      else if (s == L2_LOST) { failChoice = 0; beginScene(FAIL_MENU); }
    }
  }
  arduboy.display();
}
