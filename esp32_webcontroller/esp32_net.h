#ifndef ESP32_NET_H
#define ESP32_NET_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>
#include <HTTPClient.h>

// const char* DATA_URL = "http://192.168.0.190:3000/api/data";
const char *DATA_URL = "http://192.168.0.24:3000/api/data";

int net_post(const String &url, const String &requestData, String &response)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("WiFi is not connected"));
    return 0;
  }

  HTTPClient http;
  http.setTimeout(10000);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(requestData);
  if (httpCode == HTTP_CODE_OK)
  {
    response = http.getString();
    delayMicroseconds(1);
  }
  http.end();
  return httpCode;
}

int net_get(const String &url, String &response)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("WiFi is not connected"));
    return 0;
  }

  HTTPClient http;
  http.setTimeout(10000);
  http.begin(url);
  // http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK)
  {
    response = http.getString();
    delayMicroseconds(1);
  }
  http.end();
  return httpCode;
}

int post_data(const String &src, const String &type, const String &var, const String &val, String &response)
{
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

int post_data(String src, String type, String var, float val, String &response)
{
  return post_data(src, type, var, String(val), response);
}

int post_data(String src, String type, String var, int val, String &response)
{
  return post_data(src, type, var, String(val), response);
}

String net_status_code(int code)
{

  if (code == -1)
  {
    return "ERROR_CONNECTION_REFUSED";
  }
  else if (code == -2)
  {
    return "ERROR_SEND_HEADER_FAILED";
  }
  else if (code == -3)
  {
    return "ERROR_SEND_PAYLOAD_FAILED";
  }
  else if (code == -4)
  {
    return "ERROR_NOT_CONNECTED";
  }
  else if (code == -5)
  {
    return "ERROR_CONNECTION_LOST";
  }
  else if (code == -6)
  {
    return "ERROR_NO_STREAM";
  }
  else if (code == -7)
  {
    return "ERROR_NO_HTTP_SERVER";
  }
  else if (code == -8)
  {
    return "ERROR_TOO_LESS_RAM";
  }
  else if (code == -9)
  {
    return "ERROR_ENCODING";
  }
  else if (code == -10)
  {
    return "ERROR_STREAM_WRITE";
  }
  else if (code == -11)
  {
    return "ERROR_READ_TIMEOUT";
  }
  else if (code == 100)
  {
    return "HTTP_CODE_CONTINUE";
  }
  else if (code == 101)
  {
    return "HTTP_CODE_SWITCHING_PROTOCOLS";
  }
  else if (code == 200)
  {
    return "HTTP_CODE_OK";
  }
  else if (code == 201)
  {
    return "CREATED";
  }
  else if (code == 202)
  {
    return "ACCEPTED";
  }
  else if (code == 204)
  {
    return "NO_CONTENT";
  }
  else if (code == 304)
  {
    return "NOT_MODIFIED";
  }
  else if (code == 400)
  {
    return "BAD_REQUEST";
  }
  else if (code == 401)
  {
    return "UNAUTHORIZED";
  }
  else if (code == 403)
  {
    return "FORBIDDEN";
  }
  else if (code == 404)
  {
    return "NOT_FOUND";
  }
  else if (code == 405)
  {
    return "METHOD_NOT_ALLOWED";
  }
  else if (code == 408)
  {
    return "REQUEST_TIMEOUT";
  }
  else if (code == 409)
  {
    return "CONFLICT";
  }
  else if (code == 410)
  {
    return "GONE";
  }
  else if (code == 411)
  {
    return "LENGTH_REQUIRED";
  }
  else if (code == 412)
  {
    return "PRECONDITION_FAILED";
  }
  else if (code == 413)
  {
    return "REQUEST_ENTITY_TOO_LARGE";
  }
  else if (code == 414)
  {
    return "REQUEST_URI_TOO_LONG";
  }
  else if (code == 415)
  {
    return "UNSUPPORTED_MEDIA_TYPE";
  }
  else if (code == 416)
  {
    return "REQUESTED_RANGE_NOT_SATISFIABLE";
  }
  else if (code == 417)
  {
    return "EXPECTATION_FAILED";
  }
  else if (code == 500)
  {
    return "INTERNAL_SERVER_ERROR";
  }
  else if (code == 501)
  {
    return "NOT_IMPLEMENTED";
  }
  else if (code == 502)
  {
    return "BAD_GATEWAY";
  }
  else if (code == 503)
  {
    return "SERVICE_UNAVAILABLE";
  }
  else if (code == 504)
  {
    return "GATEWAY_TIMEOUT";
  }
  else if (code == 505)
  {
    return "HTTP_VERSION_NOT_SUPPORTED";
  }
  else if (code == 506)
  {
    return "VARIANT_ALSO_NEGOTIATES";
  }
  else if (code == 507)
  {
    return "INSUFFICIENT_STORAGE";
  }
  else if (code == 508)
  {
    return "LOOP_DETECTED";
  }
  else if (code == 510)
  {
    return "NOT_EXTENDED";
  }
  else if (code == 511)
  {
    return "NETWORK_AUTHENTICATION_REQUIRED";
  }
  else
  {
    return "UNKNOWN";
  }
}

#endif