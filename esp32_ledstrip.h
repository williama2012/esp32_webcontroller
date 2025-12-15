#ifndef ESP32_LEDSTRIP
#define ESP32_LEDSTRIP_H
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"

#define LED_PIN 22
#define LED_COUNT 484

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t flip = 1;
uint8_t color_weight = 1;
uint8_t color_r = 0;
uint8_t color_g = 0;
uint8_t color_b = 0;
bool color_up = true;

void BeginStrip() {
  strip.begin();
  strip.setBrightness(255);
  strip.show();
}

void set_brightness(uint8_t percent) {
  uint8_t val = map(percent, 0, 100, 0, 255);
  strip.setBrightness(val);
}

void set_pixel(uint8_t i, uint32_t color) {
  strip.setPixelColor(i, color);
  strip.show();
}

void set_pixel(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color = strip.Color(r, g, b);
  set_pixel(i, color);
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

void setLEDMatrix(uint32_t pixels[]) {
  uint32_t length = sizeof(pixels) / sizeof(pixels[0]);

  Println("Matrix - " + String(length));

  for(int i = 0; i < LED_COUNT; i++) {
    uint32_t color = pixels[i];
    strip.setPixelColor(i, color);
  }  
  strip.show();
}

void setAllColor(uint32_t color) {
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void setAllColor(uint8_t red, uint8_t green, uint8_t blue) {
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, red, green, blue);
  }
  strip.show();
}

void setAllColorSequence(uint8_t red, uint8_t green, uint8_t blue, uint32_t wait) {
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, red, green, blue);
    strip.show();
    delay(wait);
  }
}

#pragma region Effects

void snowflakes(uint8_t wait) {
  // Setup the pixel array
  int pixel[LED_COUNT];
  for(int p = 0; p < LED_COUNT; p++){
    pixel[p] = random(0, 255); 
  }
  
  // Run some snowflake cycles
  for (int j=0; j < 200; j++) {
    // Every five cycles, light a new pixel
    if((j%5)==0){
      strip.setPixelColor(random(0,60), 255,255,255);
    }
    
    // Dim all pixels by 10
    for(int p=0; p < LED_COUNT; p++){
      strip.setPixelColor(p, pixel[p],pixel[p],pixel[p] );
      pixel[p] =  pixel[p] - 10;
    }
    strip.show();
    delay(wait);
  }
}

void rainbow(int wait) {
  // 5 cycles of all colors on wheel
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    strip.rainbow(firstPixelHue);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

#pragma endregion Effects


#endif