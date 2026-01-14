#ifndef ESP32_LCD
#define ESP32_LCD_H
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ROWS 4
#define LCD_COLS 20

LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);
bool lcd_initialized = false;

void lcd_init() {
  lcd.init();
  lcd.backlight();
  lcd_initialized = true;
}

void lcd_clear() {
  if (!lcd_initialized) {
    return;
  }

  lcd.clear();
}

void lcd_print(String txt, uint8_t row = 0, uint8_t col = 0) {
  if (!lcd_initialized) {
    return;
  }

  lcd.setCursor(col, row);
  lcd.print(txt);
}

void lcd_print_r(String txt, uint8_t row = 0, uint8_t offset = 0) {
  if (!lcd_initialized) {
    return;
  }

  lcd.setCursor(LCD_COLS - txt.length() - offset, row);
  lcd.print(txt);
}

void lcd_backlight(bool on) {
  if (!lcd_initialized) {
    return;
  }

  if (on) {
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}

#endif