
#ifndef ESP32_DHT_H
#define ESP32_DHT_H
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
//#include "esp32_helpers.h"

#define DHTPIN    18
#define DHTTYPE   DHT11

DHT_Unified dht(DHTPIN, DHTTYPE);

void dht_getvalues(float& temp, float& humidity) {
  sensors_event_t event;

  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature)) {
    temp = (event.temperature * 1.8) + 32;
  }

  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity)) {
    humidity = event.relative_humidity;
  }

}

#endif
