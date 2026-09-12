#include <Arduboy2.h>
#include <ArduboyTones.h> 
#include "artemide.h"


Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);
hero player;

uint8_t time = 0;
uint8_t time_buf = 0;
uint8_t status = 0;
uint8_t height = 48;
/***
 * \detail 0 - Меню, 1 - Игра, 2 - Экран выхода
 */
uint8_t gameState = 0;
uint8_t menuSelection = 0;
uint8_t currentLevel = 1;


void setup() {
  arduboy.begin();
  arduboy.invert(true);
  arduboy.setFrameRate(60);
  level1Init(player);
  sound.tones(interstellar_theme);
}


void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }
  arduboy.pollButtons();
  arduboy.clear();
  if (gameState == 0) {
    if (!sound.playing()) {
      sound.tones(interstellar_theme);
    }
    arduboy.drawRect(0, 0, 128, 64, WHITE);
    arduboy.setCursor(35, 10);
    arduboy.print("ARTEMIDA 2");
    
    arduboy.setCursor(45, 30);
    arduboy.print("START");
    arduboy.setCursor(45, 45);
    arduboy.print("EXIT");

    if (menuSelection == 0) {
      arduboy.setCursor(32, 30);
      arduboy.print(">");
    } else {
      arduboy.setCursor(32, 45);
      arduboy.print(">");
    }

    if (arduboy.justPressed(UP_BUTTON)) {
      menuSelection = 0;
    }
    if (arduboy.justPressed(DOWN_BUTTON)) {
      menuSelection = 1;
    }

    if (arduboy.justPressed(A_BUTTON)) {
      sound.noTone();
      if (menuSelection == 0) {
        currentLevel = 1;
        level1Init(player);
        gameState = 1;
      } else {
        gameState = 2;
      }
    }
  } else if (gameState == 1){

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
      } else if (s == 2 && arduboy.justPressed(A_BUTTON)) {
        level1Init(player);
        currentLevel = 1;
        gameState = 0;
      }
    } else {
      level2Update(player);
      level2Draw(player);

      uint8_t s = level2State();
      if (s == 1 && arduboy.justPressed(A_BUTTON)) {
        currentLevel = 2;
        level2Init(player);
        gameState = 0;
      } else if (s == 2 && arduboy.justPressed(A_BUTTON)) {
        level1Init(player);
        //currentLevel = 1;
        //gameState = 0;
      }
    }
  } else if (gameState == 2) {
    arduboy.setCursor(20, 25);
    arduboy.print("GAME OVER / BYE!");
    arduboy.setCursor(15, 45);
    arduboy.print("Press B to Menu");

    if (arduboy.justPressed(B_BUTTON)) {
      gameState = 0;
    }
  }
  arduboy.display();
}
