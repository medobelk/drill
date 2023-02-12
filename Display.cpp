#include "Display.h"
#include <Arduino.h>

#include <stdint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

short animationIteration = 1;

void renderNumber(int horizontalPosition, int number) {
  switch (number) {
    case 0:
      lcd.setCursor(horizontalPosition, 0); 
      lcd.write(8);  
      lcd.write(1); 
      lcd.write(2);
      lcd.setCursor(horizontalPosition, 1); 
      lcd.write(3);  
      lcd.write(4);  
      lcd.write(5);
    break;
    case 1:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(horizontalPosition + 1, 1);
      lcd.write(255);
    break;
    case 2:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(horizontalPosition, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(7);
    break;
    case 3:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(horizontalPosition, 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5); 
    break;
    case 4:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(3);
      lcd.write(4);
      lcd.write(2);
      lcd.setCursor(horizontalPosition + 2, 1);
      lcd.write(255);
    break;
    case 5:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(255);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(horizontalPosition, 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
    break;
    case 6:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(8);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(horizontalPosition, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
    break;
    case 7:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(1);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(horizontalPosition + 1, 1);
      lcd.write(8);
    break;
    case 8:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(8);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(horizontalPosition, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
    break;
    case 9:
      lcd.setCursor(horizontalPosition, 0);
      lcd.write(8);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(horizontalPosition + 2, 1);
      lcd.write(255);
    break;
  }
}

void setAnimation() {
  switch (animationIteration) {
    case 1:
      lcd.setCursor(0, 0);
      lcd.write(29);
      lcd.setCursor(0, 1);
      lcd.write(29);
    break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.write(29);
      lcd.setCursor(0, 1);
      lcd.write(29);
      lcd.setCursor(1, 0);
      lcd.write(29);
      lcd.setCursor(1, 1);
      lcd.write(29);
    break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.write(29);
      lcd.setCursor(0, 1);
      lcd.write(29);
      lcd.setCursor(1, 0);
      lcd.write(29);
      lcd.setCursor(1, 1);
      lcd.write(29);
      lcd.setCursor(2, 0);
      lcd.write(29);
      lcd.setCursor(2, 1);
      lcd.write(29);
    break;
    case 4:
      lcd.setCursor(1, 0);
      lcd.write(29);
      lcd.setCursor(1, 1);
      lcd.write(29);
      lcd.setCursor(2, 0);
      lcd.write(29);
      lcd.setCursor(2, 1);
      lcd.write(29);
    break;
    case 5:
      lcd.setCursor(2, 0);
      lcd.write(29);
      lcd.setCursor(2, 1);
      lcd.write(29);
    break;
    case 6:
      lcd.setCursor(2, 0);
      lcd.write(28);
      lcd.setCursor(2, 1);
      lcd.write(28);
    break;
    case 7:
      lcd.setCursor(2, 0);
      lcd.write(28);
      lcd.setCursor(2, 1);
      lcd.write(28);
      lcd.setCursor(1, 0);
      lcd.write(28);
      lcd.setCursor(1, 1);
      lcd.write(28);
    break;
    case 8:
      lcd.setCursor(2, 0);
      lcd.write(28);
      lcd.setCursor(2, 1);
      lcd.write(28);
      lcd.setCursor(1, 0);
      lcd.write(28);
      lcd.setCursor(1, 1);
      lcd.write(28);
      lcd.setCursor(0, 0);
      lcd.write(28);
      lcd.setCursor(0, 1);
      lcd.write(28);
    break;
    case 9:
      lcd.setCursor(1, 0);
      lcd.write(28);
      lcd.setCursor(1, 1);
      lcd.write(28);
      lcd.setCursor(0, 0);
      lcd.write(28);
      lcd.setCursor(0, 1);
      lcd.write(28);
    break;
    case 10:
      lcd.setCursor(0, 0);
      lcd.write(28);
      lcd.setCursor(0, 1);
      lcd.write(28);
      animationIteration = -1;
    break;
  }

  animationIteration++;
}

void setNumber(int number) {
  int startCursorPosition = 13;
  int horizontalPosition, verticalPosition;
  int length, lastDigit, symbolIndex = 1;

  if ((float) number / 10 < 1) {
    length = 1;
  } else if ((float) number / 100 < 1) {
    length = 2;
  } else if ((float) number / 1000 < 1) {
    length = 3;
  } else {
    length = 4;
  }

  for (int i = 0; i < length; i++) {
    lastDigit = number % 10;
    number = int((float)number / 10);

    renderNumber(startCursorPosition, lastDigit);

    startCursorPosition = startCursorPosition - 3;
  }
}

void displayRender(int number) {
  lcd.clear();
  setAnimation();
  setNumber(number);
}

void displayInit() {
  byte LT[8] = {B00111, B01111, B11111, B11111, B11111, B11111, B11111, B11111};
  byte UB[8] = {B11111, B11111, B11111, B00000, B00000, B00000, B00000, B00000};
  byte RT[8] = {B11100, B11110, B11111, B11111, B11111, B11111, B11111, B11111};
  byte LL[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B01111, B00111};
  byte LB[8] = {B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111};
  byte LR[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11110, B11100};
  byte UMB[8] = {B11111, B11111, B11111, B00000, B00000, B00000, B11111, B11111};
  byte LMB[8] = {B11111, B00000, B00000, B00000, B00000, B11111, B11111, B11111};

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.createChar(8,LT); //addresses 0-7 for custom symbols 8-15 duplicate them
  lcd.createChar(1,UB);
  lcd.createChar(2,RT);
  lcd.createChar(3,LL);
  lcd.createChar(4,LB);
  lcd.createChar(5,LR);
  lcd.createChar(6,UMB);
  lcd.createChar(7,LMB);
}
