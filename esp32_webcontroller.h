#ifndef ESP32_CONTROLLER
#define ESP32_CONTROLLER_H
#include <Arduino.h>
#include "esp32_timer.h"
#include "secrets.h"
//#include "Servo.h"
#include "incbin.h"
#include "Wire.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LiquidCrystal_I2C.h>

#define SERIAL_BAUDRATE 115200

//enum PinModeEnum {AnalogWrite, AnalogRead, DigitalWrite, DigitalRead, Servo, Tone};

typedef struct {
  int pin;
  int pos;
  bool isNew;
} servoPosDef;

typedef struct {
  int pin;
  int value;
  int mode; // TODO - AnalogWrite, AnalogRead, DigitalWrite, DigitalRead, Servo, Tone
} PinMode;

String jsonField(String field, String value, bool addMore) {
  return "\"" + field + "\":" + "\"" + value + "\"" + (addMore ? "," : "");
}

String makeStatusItem(int pin, String message, bool includeComma = true) {
  String msg = "{" + jsonField("pin", String(pin), true) + jsonField("status", message, false) + "}";
  if (includeComma) {
    msg += ",";
  }
  return msg;
}








#endif