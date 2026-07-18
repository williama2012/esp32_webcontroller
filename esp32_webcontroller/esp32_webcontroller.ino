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
  //timers.AddTimer(0, 100);  // Network Counts
  //timers.AddTimer(1, 3000); // Wifi Signal

}

void PreSetup() {
  #ifdef ESP32_LCD_H
    lcd_init();
  #endif
  
  #ifdef ESP32_LED_H
    setup_leds();
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
    //set_pixel(11, 11, 255, 255, 255);
  #endif

  reset_counters();
}

void SetupPins() {

}
  
#pragma endregion Setup

#pragma region Testing

float* temps;
float temp;

#pragma endregion Testing


// Runs on Core 1
void loop(void) {
  #ifdef ESP32_LED_H
    loop_leds();
  #endif

}
