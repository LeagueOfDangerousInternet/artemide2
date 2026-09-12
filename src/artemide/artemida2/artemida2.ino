#include <Arduboy2.h>
#include "artemide.h"


Arduboy2 arduboy;
hero player;

uint8_t time = 0;
uint8_t time_buf = 0;
uint8_t status = 0;
uint8_t height = 48;

uint8_t currentLevel = 1;


void setup() {
  arduboy.begin();
  arduboy.invert(true);
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
    if (s == 1 && arduboy.justPressed(A_BUTTON)) {
      currentLevel = 2;
      level2Init(player);
    }
    else if (s == 2 && arduboy.justPressed(A_BUTTON)) {
      level1Init(player);
    }
  }
  else {
    level2Update(player);
    level2Draw(player);

    uint8_t s = level2State();
    if (s == 1 && arduboy.justPressed(A_BUTTON)) {
      currentLevel = 1;
      level1Init(player);
    }
    else if (s == 2 && arduboy.justPressed(A_BUTTON)) {
      level2Init(player);
    }
  }

  arduboy.display();
}
