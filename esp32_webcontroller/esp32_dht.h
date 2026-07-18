
#ifndef ESP32_DHT_H
#define ESP32_DHT_H
#include <Arduino.h>
#include <Bonezegei_DHT11.h>
//#include <Adafruit_Sensor.h>
//#include <DHT.h>
//#include <DHT_U.h>
//#include "esp32_helpers.h"

#define DHTPIN    23
#define DHTTYPE   DHT11

//DHT_Unified dht(DHTPIN, DHTTYPE);
Bonezegei_DHT11 dht(DHTPIN);

void dht_getvalues(float& temp, float& humidity) {
  if (dht.getData()) {
    temp = dht.getTemperature(true);
    humidity= dht.getHumidity();
  }
}

#endif
