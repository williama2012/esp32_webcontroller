#ifndef ESP32_LEDSTRIP
#define ESP32_LEDSTRIP_H
#include <Arduino.h>
#include <FastLED.h>

// Configuration
#define LED_PIN     22
#define NUM_LEDS    484
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

#define LED_COUNT 484

CRGB leds[NUM_LEDS];


uint8_t flip = 1;
uint16_t color_weight = 1;
uint16_t color_r = 0;
uint16_t color_g = 0;
uint16_t color_b = 0;
bool color_up = true;


// Params for width and height
const uint16_t kMatrixWidth = 22;
const uint16_t kMatrixHeight = 22;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = true;
const bool    kMatrixVertical = false;

uint16_t XY( uint8_t x, uint8_t y)
{
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







void led_clear() {
  fill_solid(leds, NUM_LEDS, CRGB(0,0,0));
  FastLED.show();  
}


void BeginStrip() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(100);
    led_clear();
}

void set_brightness(uint16_t brightness) {
  FastLED.setBrightness(brightness);
}

void set_brightnessP(uint16_t percent) {
  uint16_t val = map(percent, 0, 100, 0, 255);
  FastLED.setBrightness(val);
}

void set_pixel(uint16_t i, CRGB color) {
  if (i >= LED_COUNT) {
    i = LED_COUNT - 1;
  }
  
  Println("set_pixel:" + String(i));

  leds[i] = color;
  FastLED.show();
}

void set_pixel(uint16_t x, uint16_t y, CRGB color = CRGB::White) {

  uint16_t i = XY(x, y);

  Println(String(x) + "," + String(y) + ":" + String(i));

  set_pixel(i, color);
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
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

void setAllColor(uint16_t r, uint16_t g, uint16_t b) {
  setAllColor(CRGB(r, g, b));
}

void setAllColorSequence(uint16_t red, uint16_t green, uint16_t blue, uint32_t wait = 10) {
  for(int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB(red, green, blue);
    FastLED.show();
    delay(wait);
  }
}


void setLEDMatrix(uint32_t pixels[]) {
  uint32_t length = sizeof(pixels) / sizeof(pixels[0]);

  Println("Matrix - " + String(length));

  for(int i = 0; i < LED_COUNT; i++) {
    uint32_t color = pixels[i];
    //strip.setPixelColor(i, color);
  }  
  //strip.show();
}






#pragma region Effects

// void snowflakes(uint8_t wait) {
//   // Setup the pixel array
//   int pixel[LED_COUNT];
//   for(int p = 0; p < LED_COUNT; p++){
//     pixel[p] = random(0, 255); 
//   }
  
//   // Run some snowflake cycles
//   for (int j=0; j < 200; j++) {
//     // Every five cycles, light a new pixel
//     if((j%5)==0){
//       strip.setPixelColor(random(0,60), 255,255,255);
//     }
    
//     // Dim all pixels by 10
//     for(int p=0; p < LED_COUNT; p++){
//       strip.setPixelColor(p, pixel[p],pixel[p],pixel[p] );
//       pixel[p] =  pixel[p] - 10;
//     }
//     strip.show();
//     delay(wait);
//   }
// }

// void rainbow(int wait) {
//   // 5 cycles of all colors on wheel
//   for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
//     strip.rainbow(firstPixelHue);
//     strip.show(); // Update strip with new contents
//     delay(wait);  // Pause for a moment
//   }
// }

#pragma endregion Effects


#endif