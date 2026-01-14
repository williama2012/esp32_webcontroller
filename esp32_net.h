#ifndef ESP32_NET
#define ESP32_NET_H
#include <Arduino.h>

#include <HTTPClient.h>
#include <UrlEncode.h>

const char* DATA_URL = "http://192.168.0.190:3000/data";

String net_post(String url, String requestData) {
  Serial.print("REQUEST:");
  Serial.println(url);
  Serial.print("BODY:");
  Serial.println(requestData);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST(requestData);
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.print("RESPONSE:");
      Serial.println(payload);
      return payload;
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

    return net_post(url, "{\"src\":\"test\"}");
}


#endif