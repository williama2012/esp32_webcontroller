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

TaskHandle_t Task1;
WebServer server(80);
//Servo servo_ctrl;
LiquidCrystal_I2C lcd(0x27, 20, 2);

INCTXT(WebPage, "index.html");
INCTXT(WebJavascript, "index.js");
INCTXT(WebStylesheet, "index.css");

String url;
bool doBlink = false;
int servo_pin = 0;
//servoPosDef servoPos;


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


#pragma region Printing

void Print(String msg) {
  if (Serial) {
    Serial.print(msg);
  }
}

void Print(const char *msg) {
  if (Serial) {
    Serial.print(msg);
  }
}

void Print(char *msg) {
  if (Serial) {
    Serial.print(msg);
  }
}

void Print(uint32_t msg) {
  if (Serial) {
    Serial.print(msg);
  }
}

void Println(String msg) {
  if (Serial) {
    Serial.println(msg);
  }
}

void Println(const char *msg) {
  if (Serial) {
    Serial.println(msg);
  }
}

void Println(char *msg) {
  if (Serial) {
    Serial.println(msg);
  }
}

void Println(uint32_t msg) {
  if (Serial) {
    Serial.println(msg);
  }
}

void PrintCore(char *msg) {
  Print("--- " + String(msg) + " running on core ");
  Print(xPortGetCoreID());
  Println(" ---");
}

#pragma endregion Printing










#endif