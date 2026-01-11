#ifndef ESP32_LCD
#define ESP32_LCD_H
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 4);

void LcdInit() {
  lcd.init();
  lcd.backlight();
}

void LcdClear() {
  lcd.clear();
}

void LcdPrint(String txt, uint8_t row = 0, uint8_t col = 0) {
  lcd.setCursor(row, col);
  lcd.print(txt);
}



#endif