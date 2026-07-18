#include "esp32_webcontroller.h"

uint8_t mode = 1;
bool show_rssi = false;

bool lcd_show_network = true;
bool lcd_show_rssi = true;
bool serial_debug = false;

int Wifi_Signal;
String _net_response;

#pragma region Setup

void SetupTimers() {
  timers.AddTimer(0, 100);  // Network Counts
  timers.AddTimer(1, 3000); // Wifi Signal
  timers.AddTimer(10, 101);
  timers.AddTimer(20, 501);
}

void PreSetup() {
  #ifdef ESP32_LCD_H
    lcd_init();
  #endif
  
  #ifdef ESP32_LED_H
    led_init();
  #endif
  
  #ifdef ESP32_DHT_H
    dht.begin();
  #endif

  #ifdef ESP32_ONEWIRE_H
    ds_init();
  #endif
}

void NetReady() {
  #ifdef ESP32_LED_H
    set_pixel(11, 11, 255, 255, 255);
  #endif

  reset_counters();
}

void SetupPins() {
	pinMode(13, INPUT);
  pinMode(23, INPUT);
  pinMode(25, INPUT);
  pinMode(26, INPUT);
}
  
#pragma endregion Setup

#pragma region Testing

float GetRange(uint8_t triggerPin, uint8_t echoPin) {
	digitalWrite(triggerPin, LOW);
	delayMicroseconds(2);
	digitalWrite(triggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(triggerPin, LOW);
  float duration = pulseIn(echoPin, HIGH);
  return (duration * 0.0343)/2;
}

int microwave_0;
int microwave_1;
float* temps;
float temp;

#pragma endregion Testing


uint32_t motion_0 = 0;
uint32_t motion_1 = 0;

// Runs on Core 1
void loop(void) {
  // microwave_0 = digitalRead(25);
  // if (microwave_0 == 0) {
  //   motion_0++;
  // }

  // microwave_1 = digitalRead(26);
  // if (microwave_1 == 0) {
  //   motion_1++;
  // }


  //lcd_print(String(motion_0), 0, 0);
  //lcd_print(String(motion_1), 1, 0);

  if (doBlink) {
    Blink();
    doBlink = false;
  }

  switch (mode) {
    case 1:
      break;
    default:
      break;
  }

  #ifdef ESP32_LCD_H
    if (lcd_show_network && timers.CheckTimer(0)) {
      lcd_print(String(counters[0]) + "|" + String(counters[1]) + "|" + String(counters[2]), 3);
    }
  #endif

  #ifdef ESP32_LCD_H
    if (lcd_show_rssi && timers.CheckTimer(1)) {
      Wifi_Signal = WiFi.RSSI();
      //TODO: Make specific function to not make a new String.
      lcd_print_r((String(Wifi_Signal) + " dBm"), 3);
    }
  #endif

  #ifdef ESP32_DHT_H
   #ifdef ESP32_LCD_H
    if (timers.CheckTimer(1)) {
        float temp, hum;
        dht_getvalues(temp, hum);
        lcd_print_r((String(temp) + " F"), 0);
        lcd_print_r((String(hum) + " %"), 1);
    }
    #endif
  #endif
  // if (timers.CheckTimer(21)) {
  //   lcd_print("D:" + String(GetRange(12, 13)), 0);
  //   delay(10);
  //   lcd_print("D:" + String(GetRange(26, 27)), 1);
  //   lcd_print_r(String(microwave));
  // }
}

