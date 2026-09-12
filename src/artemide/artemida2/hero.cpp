#include "artemide.h"
#include "hero.h"

/*
биты в status
0 --- зажата клавиша вниз
1 --- зажата клавиша вверх в этом кадре!
2 --- зажата клавиша вперёд
3 --- зажата клавиша назад

4 --- 
5 --- 
6 ---
7 ---


*/



hero::hero() {
	x = 16;
	y = 32;
	vx = 0;
	vy = 0;

  status = 0;
  score = 0;
}


/*
void hero::vel_x() {
  if ((status & 4) ^ (status & 8)) {
    if (status & 4) {
      if (vx > 4) {
        vx += 3;
      }
      else {
        vx += 1;
      }
      if (vx > 12) {
        vx = 12;
      }
    }

    if (status & 8) {
      if (vx < -4) {
        vx -= 3;
      }
      else {
        vx -= 1;
      }
      if (vx < -12) {
        vx = -12;
      }
    }
  }
}


void hero::vel_y() {
  if ((status & 2) && (vy == 0) && (y + 12 == height)) { // заменить на условие "и он стоит на земле", иначе баг в вершине параболы
    vy -= 20;
  }

  vy += 4;
  if (vy > 8) {
    vy = 8;
  }
  
  y += vy;
  
  if (y + 12 > height) {  // проверяем землю
    y = height;
    vy = 0;
  }
}


void hero::down() {
	// пока не нужно
}
*/

void hero::update(uint8_t status) {
  this->status = status; 
}

void hero::draw() {
  if (status & 1) {// если зажата клавиша вниз
    arduboy.drawBitmap(x, y, man_3, 16, 16, WHITE);
  }
	else if (time_buf / 10 < 5) {
		arduboy.drawBitmap(x, y, man_1, 16, 16, WHITE);
	}
	else {
		arduboy.drawBitmap(x, y, man_2, 16, 16, WHITE);
	}
}
