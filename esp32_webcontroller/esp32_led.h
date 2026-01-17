#ifndef ESP32_LED_H
#define ESP32_LED_H
#include <Arduino.h>
#include <FastLED.h>

// Configuration
#define LED_PIN     22
#define NUM_LEDS    484
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define LED_COUNT 484
CRGB leds[NUM_LEDS];

bool led_initialized = false;
uint8_t flip = 1;
uint16_t color_weight = 1;
uint16_t color_r = 0;
uint16_t color_g = 0;
uint16_t color_b = 0;
bool color_up = true;

#pragma region Pixel XY coordination

// Params for width and height
const uint16_t kMatrixWidth = 22;
const uint16_t kMatrixHeight = 22;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = true;
const bool    kMatrixVertical = false;

uint16_t XY( uint8_t x, uint8_t y) {
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    if (kMatrixVertical == false) {
      i = (y * kMatrixWidth) + x;
    } else {
      i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
    }
  }

  if( kMatrixSerpentineLayout == true) {
    if (kMatrixVertical == false) {
      if( y & 0x01) {
        // Odd rows run backwards
        uint8_t reverseX = (kMatrixWidth - 1) - x;
        i = (y * kMatrixWidth) + reverseX;
      } else {
        // Even rows run forwards
        i = (y * kMatrixWidth) + x;
      }
    } else { // vertical positioning
      if ( x & 0x01) {
        i = kMatrixHeight * (kMatrixWidth - (x+1))+y;
      } else {
        i = kMatrixHeight * (kMatrixWidth - x) - (y+1);
      }
    }
  }
  
  return i;
}

#pragma endregion Pixel XY coordination

void led_clear() {
  if (!led_initialized) { return; }
  fill_solid(leds, NUM_LEDS, CRGB(0,0,0));
  FastLED.show();
}

void led_init() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(100);
    led_initialized = true;
    led_clear();
}

void set_brightness(uint16_t brightness) {
  if (!led_initialized) { return; }

  FastLED.setBrightness(brightness);
  FastLED.show();
}

void set_brightnessP(uint16_t percent) {
  if (!led_initialized) { return; }

  uint16_t val = map(percent, 0, 100, 0, 255);
  FastLED.setBrightness(val);
  FastLED.show();
}

void set_pixel_i(uint16_t i, CRGB color, bool hold_refresh = false) {
  if (!led_initialized) { return; }

  if (i >= LED_COUNT) {
    i = LED_COUNT - 1;
  }

  leds[i] = color;
  
  if (!hold_refresh) {
    FastLED.show();
  }
}

void set_pixel_i(uint16_t i, uint16_t r, uint16_t g, uint16_t b, bool hold_refresh = false) {
  set_pixel_i(i, CRGB(r, g, b), hold_refresh);
}

void set_pixel(uint16_t x, uint16_t y, CRGB color = CRGB::White, bool hold_refresh = false) {
  uint16_t i = XY(x, y);
  set_pixel_i(i, color, hold_refresh);
}

void set_pixel(uint16_t x, uint16_t y, uint16_t r, uint16_t g, uint16_t b, bool hold_refresh = false) {
  set_pixel(x, y, CRGB(r, g, b), hold_refresh);
}

CRGB led_color(String color) {
  Serial.println(color);
  color.toLowerCase();

  if (color == "white") {
    return CRGB::White;
  }
  if (color == "red") {
    return CRGB::Red;
  }
  if (color == "green") {
    return CRGB::Green;
  }
  if (color == "blue") {
    return CRGB::Blue;
  }
  if (color == "Black") {
    return CRGB::Black;
  }

  return CRGB::Black;
}

void flip_color() {
  if (flip == 1) {
    flip = 2;
  } else {
    flip = 1;
  }
}

void step_weight() {
  if (color_up) {
    color_weight = color_weight + 1;
    if (color_weight >= 255) {
      color_weight = 255;
      color_up = false;
    }
  } else {
    color_weight = color_weight - 1;
    if (color_weight <= 1) {
      color_weight = 1;
      flip_color();
      color_up = true;
    }
  }
}

void setAllColor(CRGB color) {
  if (!led_initialized) { return; }
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

void setAllColor(uint16_t r, uint16_t g, uint16_t b) {
  setAllColor(CRGB(r, g, b));
}

void setAllColorSequence(uint16_t red, uint16_t green, uint16_t blue, uint32_t wait = 10) {
  if (!led_initialized) { return; }
  for(int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB(red, green, blue);
    FastLED.show();
    delay(wait);
  }
}

#endif