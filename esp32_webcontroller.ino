#include "esp32_webcontroller.h"
#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"
#include "WS2812_Matrix.h"

Timer timers;

#define LED_PIN 22
#define LED_COUNT 484

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t flip = 1;
uint8_t color_weight = 1;
uint8_t color_r = 0;
uint8_t color_g = 0;
uint8_t color_b = 0;
bool color_up = true;

uint8_t mode = 1;

void SetupPins() {
  pinMode(23, OUTPUT);
}

void SetupTimers() {
  timers.AddTimer(0, 1);
  timers.AddTimer(1, 10000);

  strip.begin();
  set_brightness(100);
  strip.show();      

}

void set_brightness(uint8_t percent) {
  uint8_t val = map(percent, 0, 100, 0, 255);
  strip.setBrightness(val);
}



void flip_color() {
  if (flip == 1) {
    flip = 2;
  } else {
    flip = 1;
  }
}

PinSet prev_mode;

void loop(void) {

  if (doBlink) {
    Blink();
    doBlink = false;
  }
  
  PinSet mode = get_pin(50);

  bool changed = mode.value != prev_mode.value;

  if (changed) {
    PrintCore("Mode Changed: " + String(mode.value));
  }

  switch (mode.value) {
    case 1:
      mode1process();
      break;
    case 2:
      mode2process();
      break;
    case 3:
      break;
    default:
      mode1process();
  }

  prev_mode = mode;
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

void mode2process() {
  PinSet red_set = get_pin(51);
  PinSet green_set = get_pin(52);
  PinSet blue_set = get_pin(53);
  setAllColor(red_set.value, green_set.value, blue_set.value);
}

void mode1process() {
  if (timers.CheckTimer(0)) {
    step_weight();

    if (flip == 1) {
      color_r = color_weight;
      color_g = 0;
      color_b = 0;
      setAllColor(color_r, color_g, color_b);
    } else {
      color_r = 0;
      color_g = color_weight;
      color_b = 0;
      setAllColor(0, color_weight, 0);
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

void setPixel(uint8_t x, uint8_t y, uint32_t color) {



}



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


