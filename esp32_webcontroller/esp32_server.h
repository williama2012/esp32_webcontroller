#ifndef ESP32_SERVER_H
#define ESP32_SERVER_H
#include <Arduino.h>
#include "secrets.h"
#include "Wire.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp32_helpers.h"

#define HTTP_PORT 80

WebServer server(HTTP_PORT);


#pragma region server

int intArg(String name) {
  String strVal = server.arg(name);
  int val = strVal.toInt();
  return val;
}

bool send_body(String body) {
  server.send(200, "application/json", "{" + body + "}");
  return true;
}

bool send_rec(String cmd) {
  send_body(jsonField("received", cmd, false));
  return true;
}

bool send_rec() {
  send_rec(jsonField("received", "command", false));
  return true;
}

bool send_msg(String msg) {
  send_body(jsonField("msg", msg, false));
  return true;
}

bool send_500(String msg) {
  server.send(500, "application/json", "{" + jsonField("msg", msg, false) + "}");
  return true;
}

#pragma endregion server

#endif