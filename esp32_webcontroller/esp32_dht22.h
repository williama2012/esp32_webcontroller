
#ifndef ESP32_DHT22_H
#define ESP32_DHT22_H
#include <Arduino.h>
#include <Bonezegei_DHT22.h>

#define DHT22PIN  26

Bonezegei_DHT22 dht(DHT22PIN);

void dht_getvalues(float& temp, float& humidity) {
  if (dht.getData()) {                         // get All data from DHT22
    temp = dht.getTemperature(true);  // return temperature in fahrenheit if true celsius of false
    humidity = dht.getHumidity();               // return humidity
    Serial.printf("Temperature: %0.1lf°F Humidity:%d \n", temp, humidity);
  }
}

#endif
