#ifndef ESP32_LCD
#define ESP32_LCD_H
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ROWS 4
#define LCD_COLS 20

LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);

void lcd_init() {
  lcd.init();
  lcd.backlight();
}

void lcd_clear() {
  lcd.clear();
}

void lcd_print(String txt, uint8_t row = 0, uint8_t col = 0) {
  lcd.setCursor(col, row);
  lcd.print(txt);
}

void lcd_print_r(String txt, uint8_t row = 0, uint8_t offset = 0) {
  lcd.setCursor(LCD_COLS - txt.length() - offset, row);
  lcd.print(txt);
}

void lcd_backlight(bool on) {
  if (on) {
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}

#endif