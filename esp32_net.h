#ifndef ESP32_NET
#define ESP32_NET_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>

#include <HTTPClient.h>

const char* DATA_URL = "http://192.168.0.190:3000/data";

String net_post(String url, String requestData) {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi is not connected");
    return "WiFi is not connected";
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST(requestData);
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      return http.getString();
    }
  }

  return http.errorToString(httpCode).c_str();
}

String post_data(String src, String type, String var, String val) {
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

    String url = DATA_URL;
    url += "?src=" + urlEncode("01:01:01:01:01:01");
    String response = net_post(url, buffer);

    post_data(MACADDRESS, "type0", "arg0", "val_0x01");




    return net_post(url, "{\"src\":\"test\"}");
}

String post_data(String src, String type, String var, float val) {
  return post_data(src, type, var, String(val));
}

String post_data(String src, String type, String var, int val) {
  return post_data(src, type, var, String(val));
}




#endif