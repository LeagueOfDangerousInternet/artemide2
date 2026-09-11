#include <Arduboy2.h>
#include "artemide.h"

Arduboy2 arduboy;

bool robotStep = false;

void setup() {
	arduboy.begin();
  arduboy.setFrameRate(30);
  arduboy.invert(true);
}

void loop() {  
	if (!arduboy.nextFrame()) return;
  arduboy.pollButtons();
  const bool sitting = arduboy.pressed(A_BUTTON);
  // Alternate poses every 8 frames (about four steps per second).
  if (!sitting && arduboy.everyXFrames(8)) robotStep = !robotStep;
 
	arduboy.clear();

	arduboy.drawBitmap(10, 10, block, 8, 8, WHITE);
  arduboy.drawBitmap(10, 20, comp, 8, 8, WHITE);
  arduboy.drawBitmap(10, 30, nile_1, 8, 8, WHITE);
  arduboy.drawBitmap(10, 40, uno_1, 8, 8, WHITE);
  arduboy.drawBitmap(10, 50, uno_2, 8, 8, WHITE);
  arduboy.drawBitmap(10, 60, uno_3, 8, 8, WHITE);
  if (sitting) {
    arduboy.drawBitmap(30, 30, man_sit, 16, 16, WHITE);
  }
  else if (robotStep) {
    arduboy.drawBitmap(30, 30, man_1, 16, 16, WHITE);
  }
  else {
    arduboy.drawBitmap(30, 30, man_2, 16, 16, WHITE);
  }

	arduboy.display();
}
