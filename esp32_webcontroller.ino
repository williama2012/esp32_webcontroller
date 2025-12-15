#include "esp32_webcontroller.h"
#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"
#include "WS2812_Matrix.h"

Timer timers;

#define LED_PIN 22
#define LED_COUNT 484

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void SetupPins() {
  pinMode(23, OUTPUT);
}

void SetupTimers() {
  timers.AddTimer(0, 3000);

  strip.begin();           // Initialize NeoPixel object
  strip.setBrightness(128); // Set BRIGHTNESS (max = 255)
  strip.show();      
}

#pragma region servo

// void MoveServo(int pin, int pos) {
//   PrintCore("MoveServo");

//   if (pin != servo_pin) {
//     if (servo_pin != 0) {
//       servo_ctrl.detach(servo_pin);
//     }
//     servo_ctrl.attach(pin);
//     servo_pin = pin;
//   }

//   servo_ctrl.write(pos);
// }

// void ClearServo(int pin) {
//   if (servo_pin == pin) {
//     servo_ctrl.detach(servo_pin);
//     servo_pin = 0;
//   }
// }

#pragma endregion servo

void Core0Processor(void *parameter) {
  PrintCore("Core0Processor");

  for (;;) {
    server.handleClient();
  }
}

uint32_t flip = 1;

void loop(void) {

  if (doBlink) {
    Blink();
    doBlink = false;
  }

  if (timers.CheckTimer(0)) {
    //Blink();
    //LcdUpdateRows();
  }


  if (flip == 1) {
    Println("Red");
    setAllColor(255, 0, 0);
    flip = 2;
  } else if (flip == 2) {
    Println("Green");
    setAllColor(0, 255, 0);
    flip = 3;
  } else if (flip == 3) {
    Println("Blue");
    setAllColor(0, 0, 255);
    flip = 4;
  } else if (flip == 4) {
    Println("Rainbow");
    rainbow(10);
    flip = 5;
  } else {
    Println(flip);

    setLEDMatrix(FullBlack);
    delay(3000);

    flip = 1;
  }
  //rainbow(10);
  //delayMicroseconds(1);
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


void setAllColor(uint8_t red, uint8_t green, uint8_t blue) {
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, red, green, blue);
    strip.show();
    delay(10);
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


