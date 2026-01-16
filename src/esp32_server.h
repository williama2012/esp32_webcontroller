#ifndef ESP32_HELPERS
#define ESP32_HELPERS_H
#include <Arduino.h>
#include "secrets.h"
#include "incbin.h"
#include "Wire.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp32_helpers.h"

#define HTTP_PORT 80

WebServer server(HTTP_PORT);

INCTXT(WebPage, "index.html");
INCTXT(TerminalWebPage, "terminal.html");
INCTXT(MatrixWebPage, "matrix.html");
INCTXT(WebJavascript, "index.js");
INCTXT(WebStylesheet, "index.css");

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
  return send_body(jsonField("status", "500", true) + jsonField("msg", msg, false));
}

#pragma endregion server

#endif