#ifndef ESP32_LCD
#define ESP32_LCD_H
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ROWS 4
#define LCD_COLS 20

byte HappyFace[] = { B00000, B01010, B01010, B00000, B10001, B01110, B00000, B00000 };
byte SadFace[] = { B10001, B11011, B01110, B00100, B00100, B01110, B11011, B10001 };

LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);
bool lcd_initialized = false;

void lcd_init() {
  lcd.init();
  lcd.backlight();
  lcd.noCursor();
  lcd.noBlink();

  lcd.createChar(0, HappyFace);
  lcd.createChar(1, SadFace);
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

void lcd_write(uint8_t char_id, uint8_t row = 0, uint8_t col = 0) {
  if (!lcd_initialized) {
    return;
  }

  lcd.setCursor(col, row);
  lcd.write(char_id);
}

void lcd_write_r(uint8_t char_id, uint8_t row = 0, uint8_t offset = 0) {
  if (!lcd_initialized) {
    return;
  }

  lcd.setCursor(LCD_COLS - 1 - offset, row);
  lcd.write(char_id);
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