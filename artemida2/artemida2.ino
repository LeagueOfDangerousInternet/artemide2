#include <Arduboy2.h>
#include <EEPROM.h>
#include "artemide.h"
#include "cat.h"


Arduboy2 arduboy;
ArduboyTones tones(arduboy.audio.enabled);
hero player;


uint8_t time = 0;
uint8_t time_buf = 0;
uint8_t status = 0;
uint8_t height = 48;

uint8_t currentLevel = 1;

#define EEPROM_ADDR_UNLOCKED 0
#define EEPROM_ADDR_L3       1
#define EEPROM_MAGIC         0xA5

#define SCENE_MENU   0
#define SCENE_INTRO  1
#define SCENE_PLAY   2
#define SCENE_EXIT   3
#define SCENE_STORY  4

#define STORY_BEFORE_L3  0
#define STORY_HUG        1

uint8_t scene = SCENE_MENU;
uint8_t menuChoice = 0;
uint8_t introStep = 0;
uint8_t introMax = 3;
uint8_t storyId = 0;
uint8_t storyStep = 0;
uint16_t storyFrame = 0;
bool level2Unlocked = false;
bool level3Unlocked = false;
bool musicPlaying = false;

#define MENU_MAX_ITEMS 4
uint8_t menuActions[MENU_MAX_ITEMS];


static void printCenteredF(const __FlashStringHelper *s, uint8_t y) {
  uint8_t len = strlen_P((const char *)s);
  int16_t w = (int16_t)len * 6 - 1;
  if (w < 0) {
    w = 0;
  }
  int16_t x = (128 - w) / 2;
  if (x < 0) {
    x = 0;
  }
  arduboy.setCursor(x, y);
  arduboy.print(s);
}


static uint8_t buildMenu() {
  uint8_t n = 0;
  menuActions[n] = 1;
  n++;
  if (level2Unlocked) {
    menuActions[n] = 2;
    n++;
  }
  if (level3Unlocked) {
    menuActions[n] = 3;
    n++;
  }
  menuActions[n] = 0;
  n++;
  return n;
}


static void loadProgress() {
  level2Unlocked = (EEPROM.read(EEPROM_ADDR_UNLOCKED) == EEPROM_MAGIC);
  level3Unlocked = (EEPROM.read(EEPROM_ADDR_L3) == EEPROM_MAGIC);
}


static void saveProgressLevel2() {
  EEPROM.update(EEPROM_ADDR_UNLOCKED, EEPROM_MAGIC);
  level2Unlocked = true;
}


static void saveProgressLevel3() {
  EEPROM.update(EEPROM_ADDR_L3, EEPROM_MAGIC);
  level3Unlocked = true;
}


static void startLevel(uint8_t level) {
  currentLevel = level;
  if (level == 1) {
    level1Init(player);
    tones.tones(gravity);
  }
  else if (level == 2) {
    level2Init(player);
    tones.tones(interstellar);
  }
  else {
    level3Init(player);
    tones.tones(gravity);
  }
  musicPlaying = true;
  scene = SCENE_PLAY;
}


static void startIntro(uint8_t level) {
  currentLevel = level;
  introStep = 0;
  introMax = (level == 3) ? 4 : 3;
  scene = SCENE_INTRO;
}


static void startStory(uint8_t id) {
  storyId = id;
  storyStep = 0;
  storyFrame = 0;
  scene = SCENE_STORY;
}


static void stopMusicOnEnd(uint8_t s) {
  if ((s == 1 || s == 2) && musicPlaying) {
    tones.noTone();
    musicPlaying = false;
  }
}


static void drawMenu() {
  printCenteredF(F("COSMIC ROBOT"), 4);

  uint8_t count = buildMenu();
  uint8_t y = 22;
  const uint8_t selX = 20;
  const uint8_t textX = 32;

  for (uint8_t i = 0; i < count; i++) {
    arduboy.setCursor(selX, y);
    if (i == menuChoice) {
      arduboy.print(F(">"));
    }

    arduboy.setCursor(textX, y);

    uint8_t a = menuActions[i];
    if (a == 0) {
      arduboy.print(F("EXIT"));
    }
    else {
      arduboy.print(F("LEVEL "));
      arduboy.print(a);
    }
    y += 12;
  }

  printCenteredF(F("UP/DOWN   A:SELECT"), 56);

  if (arduboy.justPressed(UP_BUTTON)) {
    if (menuChoice == 0) {
      menuChoice = count - 1;
    }
    else {
      menuChoice = menuChoice - 1;
    }
  }
  if (arduboy.justPressed(DOWN_BUTTON)) {
    menuChoice = menuChoice + 1;
    if (menuChoice >= count) {
      menuChoice = 0;
    }
  }
  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
    uint8_t a = menuActions[menuChoice];
    if (a == 0) {
      scene = SCENE_EXIT;
    }
    else {
      startIntro(a);
    }
  }
}


static void drawIntro() {
  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
    introStep++;
    if (introStep >= introMax) {
      startLevel(currentLevel);
      return;
    }
  }

  if (currentLevel == 1) {
    if (introStep == 0) {
      printCenteredF(F("YOU MUST FLY"), 22);
      printCenteredF(F("TO SPACE"), 34);
    }
    else if (introStep == 1) {
      printCenteredF(F("WHY?"), 8);
      printCenteredF(F("A ROBOT DOES NOT"), 26);
      printCenteredF(F("ASK, IT OBEYS"), 38);
    }
    else {
      printCenteredF(F("COLLECT MONEY FOR"), 8);
      printCenteredF(F("THE FLIGHT"), 20);
      printCenteredF(F("AVOID ZEROS AND"), 34);
      printCenteredF(F("OBSTACLES"), 46);
    }
  }
  else if (currentLevel == 2) {
    if (introStep == 0) {
      printCenteredF(F("URGENT!"), 8);
      printCenteredF(F("YANDEX LOST ITS"), 24);
      printCenteredF(F("CAT NAMED KODERUN"), 36);
    }
    else if (introStep == 1) {
      printCenteredF(F("HE PILOTED"), 6);
      printCenteredF(F("ARTEMIS 1 AND"), 16);
      printCenteredF(F("WAS CAPTURED BY"), 26);
      printCenteredF(F("BUGS ON THE FAR"), 36);
      printCenteredF(F("SIDE OF MOON"), 46);
    }
    else {
      printCenteredF(F("FIND HIM!"), 14);
      printCenteredF(F("BEWARE THE UFO"), 34);
    }
  }
  else {
    if (introStep == 0) {
      printCenteredF(F("FOUND YOU, CAT!"), 22);
      printCenteredF(F("NO MORE RUNNING"), 34);
    }
    else if (introStep == 1) {
      printCenteredF(F("COME HERE, PUSSY"), 22);
      printCenteredF(F("I MEAN FRIEND"), 34);
    }
    else if (introStep == 2) {
      printCenteredF(F("LAST ROUND"), 14);
      printCenteredF(F("GET READY"), 34);
    }
    else {
      arduboy.drawBitmap(60, 4, heart_1, 8, 8, WHITE);
      printCenteredF(F("PRESS A"), 14);
      printCenteredF(F("TO SHOOT HEART"), 24);
      printCenteredF(F("DOWN TO DUCK"), 40);
    }
  }

  printCenteredF(F("A:NEXT"), 56);
}


static void drawStory() {
  storyFrame++;

  if ((arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) && storyStep < 3) {
    storyStep++;
    storyFrame = 0;
  }

  if (storyId == STORY_BEFORE_L3) {
    if (storyStep == 0) {
      printCenteredF(F("FOUND YOU, CAT!"), 2);
      arduboy.drawFastHLine(0, 56, 128);
      arduboy.drawBitmap(20, 40, man_1, 16, 16, WHITE);
      arduboy.drawBitmap(78, 40, space_cat, 16, 16, WHITE);
      printCenteredF(F("A:NEXT"), 56);
    }
    else if (storyStep == 1) {
      printCenteredF(F("STAY BACK, ROBOT!"), 2);
      arduboy.drawFastHLine(0, 56, 128);
      arduboy.drawBitmap(20, 40, man_2, 16, 16, WHITE);
      arduboy.drawBitmap(80, 40, space_cat, 16, 16, WHITE);
      printCenteredF(F("A:NEXT"), 56);
    }
    else if (storyStep == 2) {
      printCenteredF(F("COME BACK HERE!"), 2);
      arduboy.drawFastHLine(0, 56, 128);
      arduboy.drawBitmap(28, 40, man_1, 16, 16, WHITE);
      arduboy.drawBitmap(96, 36, space_cat, 16, 16, WHITE);
      printCenteredF(F("A:NEXT"), 56);
    }
    else {
      startLevel(3);
      return;
    }
  }
  else {
    if (storyStep == 0) {
      printCenteredF(F("YOU WIN"), 2);
      arduboy.drawFastHLine(0, 56, 128);
      arduboy.drawBitmap(30, 40, space_cat, 16, 16, WHITE);
      arduboy.drawBitmap(74, 40, man_1, 16, 16, WHITE);
      printCenteredF(F("A:NEXT"), 56);
    }
    else if (storyStep == 1) {
      printCenteredF(F("HERE, LITTLE ONE"), 2);
      arduboy.drawFastHLine(0, 56, 128);
      arduboy.drawBitmap(36, 40, space_cat, 16, 16, WHITE);
      arduboy.drawBitmap(70, 40, man_1, 16, 16, WHITE);
      printCenteredF(F("A:NEXT"), 56);
    }
    else if (storyStep == 2) {
      printCenteredF(F("FRIENDS AT LAST"), 2);
      arduboy.drawFastHLine(0, 56, 128);
      arduboy.drawBitmap(50, 40, space_cat, 16, 16, WHITE);
      arduboy.drawBitmap(66, 40, man_1, 16, 16, WHITE);
      uint8_t ph = (storyFrame / 8) % 3;
      arduboy.drawBitmap(40, 26, (ph == 0) ? heart_1 : heart_2, 8, 8, WHITE);
      arduboy.drawBitmap(78, 26, (ph == 1) ? heart_1 : heart_2, 8, 8, WHITE);
      arduboy.drawBitmap(59, 22, (ph == 2) ? heart_1 : heart_2, 8, 8, WHITE);
      printCenteredF(F("A:NEXT"), 56);
    }
    else if (storyStep == 3) {
      uint16_t f = storyFrame;

      for (uint8_t i = 0; i < 12; ++i) {
        int16_t sx = (int16_t)(i * 11 + 128 - ((f / 3) % 128)) % 128;
        if (sx < 0) {
          sx += 128;
        }
        arduboy.drawPixel(sx, 4 + (i * 11) % 48);
      }

      arduboy.drawFastHLine(0, 58, 128);

      int16_t rocketY = 36 - (int16_t)(f / 2);
      if (rocketY > 36) {
        rocketY = 36;
      }

      if (rocketY > -40) {
        arduboy.fillTriangle(60, rocketY + 26, 68, rocketY + 26,
                            64, rocketY + 34 + (f % 4), WHITE);
        arduboy.fillTriangle(64, rocketY + 2, 56, rocketY + 12,
                            72, rocketY + 12, WHITE);
        arduboy.drawRect(56, rocketY + 12, 17, 14, WHITE);
        arduboy.drawCircle(64, rocketY + 18, 3, WHITE);
        arduboy.drawPixel(62, rocketY + 18, WHITE);
        arduboy.drawPixel(66, rocketY + 18, WHITE);
      }

      for (uint8_t i = 0; i < 4; ++i) {
        int16_t hx = 26 + i * 22;
        int16_t hy = 30 + (int16_t)((f / 4) % 16) - i * 2;
        arduboy.drawBitmap(hx, hy, heart_1, 8, 8, WHITE);
      }

      printCenteredF(F("TO EARTH"), 2);

      if (storyFrame > 220) {
        scene = SCENE_MENU;
        return;
      }
    }
    else {
      scene = SCENE_MENU;
      return;
    }
  }
}


static void drawExit() {
  printCenteredF(F("GOODBYE!"), 22);
  printCenteredF(F("A: BACK TO MENU"), 38);
  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
    scene = SCENE_MENU;
  }
}


static void playScene() {
  status = 0;
  if (arduboy.pressed(DOWN_BUTTON)) {
    status |= 1;
  }
  if (arduboy.justPressed(UP_BUTTON)) {
    status |= 2;
  }
  if (arduboy.pressed(RIGHT_BUTTON)) {
    status |= 4;
  }
  if (arduboy.pressed(LEFT_BUTTON)) {
    status |= 8;
  }

  player.update(status);

  if (currentLevel == 1) {
    level1Update(player);
    level1Draw(player);

    uint8_t s = level1State();
    stopMusicOnEnd(s);
    if (s == 1) {
      saveProgressLevel2();
      if (arduboy.justPressed(A_BUTTON)) {
        scene = SCENE_MENU;
      }
    }
    else if (s == 2) {
      if (arduboy.justPressed(A_BUTTON)) {
        scene = SCENE_MENU;
      }
      if (arduboy.justPressed(B_BUTTON)) {
        startLevel(currentLevel);
      }
    }
  }
  else if (currentLevel == 2) {
    level2Update(player);
    level2Draw(player);

    uint8_t s = level2State();
    stopMusicOnEnd(s);
    if (s == 1) {
      saveProgressLevel3();
      if (arduboy.justPressed(A_BUTTON)) {
        startStory(STORY_BEFORE_L3);
      }
    }
    else if (s == 2) {
      if (arduboy.justPressed(A_BUTTON)) {
        scene = SCENE_MENU;
      }
      if (arduboy.justPressed(B_BUTTON)) {
        startLevel(currentLevel);
      }
    }
  }
  else {
    level3Update(player);
    level3Draw(player);

    uint8_t s = level3State();
    stopMusicOnEnd(s);
    if (s == 1) {
      if (arduboy.justPressed(A_BUTTON)) {
        startStory(STORY_HUG);
      }
    }
    else if (s == 2) {
      if (arduboy.justPressed(A_BUTTON)) {
        scene = SCENE_MENU;
      }
      if (arduboy.justPressed(B_BUTTON)) {
        startLevel(currentLevel);
      }
    }
  }
}


void setup() {
  arduboy.begin();
  arduboy.setFrameRate(30);

  arduboy.audio.begin();

  loadProgress();

  level1Init(player);
}


void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  arduboy.pollButtons();
  arduboy.clear();

  time++;
  time_buf = time % 60;

  if (scene == SCENE_MENU) {
    drawMenu();
  }
  else if (scene == SCENE_INTRO) {
    drawIntro();
  }
  else if (scene == SCENE_PLAY) {
    playScene();
  }
  else if (scene == SCENE_STORY) {
    drawStory();
  }
  else if (scene == SCENE_EXIT) {
    drawExit();
  }

  arduboy.display();
}
