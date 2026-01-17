#ifndef ESP32_NET
#define ESP32_NET_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>
#include <HTTPClient.h>

//const char* DATA_URL = "http://192.168.0.190:3000/api/data";
const char* DATA_URL = "http://192.168.0.233:3000/api/data";

int net_post(const String& url, const String& requestData, String& response) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi is not connected"));
    return 0;
  }
  if (IPADDRESS == "") {
    Serial.println(F("IPADDRESS not set"));
    return 0;
  }

  HTTPClient http;
  http.setTimeout(10000);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(requestData);
  if (httpCode == HTTP_CODE_OK) {
    response = http.getString();
    delayMicroseconds(1);
    http.end();
  }
  return httpCode;
}

int post_data(const String& src, const String& type, const String& var, const String& val, String& response) {
    String url = DATA_URL;
    url += "?src=" + urlEncode(src);
    url += "&type=" + urlEncode(type);
    url += "&var=" + urlEncode(var);
    url += "&val=" + urlEncode(val);

    JsonDocument doc;

    doc["src"] = src;
    doc["type"] = type;
    doc["var"] = var;
    doc["val"] = val;

    String buffer;
    serializeJson(doc, buffer);

    return net_post(url, buffer, response);
}

int post_data(String src, String type, String var, float val, String& response) {
  return post_data(src, type, var, String(val), response);
}

int post_data(String src, String type, String var, int val, String& response) {
  return post_data(src, type, var, String(val), response);
}




#endif