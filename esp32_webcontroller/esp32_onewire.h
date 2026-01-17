#ifndef ESP32_ONEWIRE_H
#define ESP32_ONEWIRE_H
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 23
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void ds_init() {
  sensors.begin();
}

float * ds_temps(uint8_t count) {
  float* array = (float*)malloc(count * sizeof(float));
  sensors.requestTemperatures();

  for(int i = 0; i < count; i++) {
    float temp = sensors.getTempFByIndex(i);
    array[i] = temp;
  }
  
  return array;
}

#endif

