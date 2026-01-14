#ifndef ESP32_NET
#define ESP32_NET_H
#include <Arduino.h>

#include <HTTPClient.h>

String net_post(String url, String requestData) {
  
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(requestData);
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
      return payload;
    }
  }

  return http.errorToString(httpCode).c_str();
}



#endif