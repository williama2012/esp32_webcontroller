#ifndef ESP32_CONTROLLER_H
#define ESP32_CONTROLLER_H
#include <stdio.h>
#include <Arduino.h>
#include "esp32_helpers.h"
#include "esp32_server.h"
#include "esp32_net.h"
#include "esp32_timer.h"
//#include "esp32_site.h"
//#include "esp32_led.h"
//#include "esp32_lcd.h"
//#include "esp32_onewire.h"
//#include "esp32_dht.h"

#define SERIAL_BAUDRATE 115200
#define VERSION 20260117.10

Timer timers;
TaskHandle_t Task1;

bool doBlink = false;
String PROGMEM IPADDRESS;
String PROGMEM MACADDRESS;

bool OnApiCommand(String& cmd);
int counters[8];
/**
 0 = API Out - Success
 1 = API Out - Failure
 2 = API In
*/
void reset_counters() {
  for(int i = 0; i < 8; i++) {
    counters[i] = 0;
  }
}

#pragma region Actions

void MatrixPost(uint16_t x, uint16_t y, uint16_t r = 255, uint16_t g = 255, uint16_t b = 255, bool hold = false) {
  doBlink = true;

  #ifdef ESP32_LED_H
    set_pixel(x, y, CRGB(r, g, b), hold);
  #endif

  send_body(
    jsonField("x", String(x), true)
    + jsonField("y", String(y), true)
    + jsonField("r", String(r), true)
    + jsonField("g", String(g), true)
    + jsonField("b", String(b), false));
}

void IntegerPost(int pin, int value) {

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), false);
  response += "}";
  doBlink = true;

  server.send(200, "application/json", response);
}

void AnalogWritePost(int pin, int value) {

  pinMode(pin, OUTPUT);
  analogWrite(pin, value);

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), false);

  response += "}";
  doBlink = true;

  server.send(200, "application/json", response);
}

void DigitalWritePost(int pin, int value) {

  pinMode(pin, OUTPUT);
  digitalWrite(pin, value);

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), false);

  response += "}";
  doBlink = true;

  server.send(200, "application/json", response);
}

void ServoWritePost(int pin, int value) {
  int pos = value;
  //int pos = map(value, 0, 4095, 0, 180);

  //MoveServo(pin, pos);
  // servoPos.pin = pin;
  // servoPos.pos = pos;
  // servoPos.isNew = true;

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), true);
  response += jsonField("pos", String(pos), false);
  response += "}";
  doBlink = true;

  server.send(200, "application/json", response);
}

void AnalogReadPost() {
  String pinStr = server.arg("pins");
  pinStr.replace("[", "");
  pinStr.replace("]", "");

  Serial.println("pins:" + pinStr);

  int i = pinStr.indexOf(",");
  String part;
  int pin = 0;

  String response = "[";

  while (i > 0) {
    part = pinStr.substring(0, i);
    pin = part.toInt();
    pinStr.remove(0, i + 1);
    i = pinStr.indexOf(",");
    if (i == -1) {
      i = pinStr.length();
    }

    if (pin > 0) {
      //ClearServo(pin);
      pinMode(pin, INPUT);
      int value = analogRead(pin);

      response += "{"
                  + jsonField("pin", String(pin), true)
                  + jsonField("value", String(value), false)
                  + "}";
      if (i > 0) {
        response += ",";
      }
    }
  }
  response += "]";

  server.send(200, "application/json", response);
}

void ToneWritePost(int pin, unsigned int value) {
  pinMode(pin, OUTPUT);

  if (value == 0) {
    noTone(pin);
  } else {
    noTone(pin);
    tone(pin, value);
  }

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), false);
  response += "}";
  doBlink = true;

  server.send(200, "application/json", response);
}

void PulsePost(int pin, int value, int time) {
  PrintCore("PulsePost");
  pinMode(pin, OUTPUT);

  analogWrite(pin, value);
  delayMicroseconds(time);
  analogWrite(pin, 0);

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), true);
  response += jsonField("time", String(time), false);
  response += "}";

  doBlink = true;

  server.send(200, "application/json", response);
}

#pragma endregion Actions

#pragma region servo

// void MoveServo(int pin, int pos) {
//   PrintCore("MoveServo");

//   if (pin != servo_pin) {
//     if (servo_pin != 0) {
//       servo_ctrl.detach(servo_pin);
//     }
//     servo_ctrl.attach(pin);
//     servo_pin = pin;
//   }

//   servo_ctrl.write(pos);
// }

// void ClearServo(int pin) {
//   if (servo_pin == pin) {
//     servo_ctrl.detach(servo_pin);
//     servo_pin = 0;
//   }
// }

#pragma endregion servo

#pragma region Get_Handlers

#ifdef ESP32_SITE_H
void handleGetIndex() {
  server.send(200, "text/html", gWebPageData);
}

void handleGetMatrix() {
  server.send(200, "text/html", gMatrixWebPageData);
}


void handleGetJavascript() {
  server.send(200, "text/javascript", gWebJavascriptData);
}

void handleGetStylesheet() {
  server.send(200, "text/css", gWebStylesheetData);
}
#endif

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void handleGetId() {
  PrintCore("handleGetId");
  server.send(200, "text/plain", MACADDRESS);
}

#pragma endregion Get_Handlers

#pragma region Post_Handlers

void handleMatrixPost() {
  counters[2]++;
  int x = server.arg("x").toInt();
  int y = server.arg("y").toInt();

  int r = server.arg("r").toInt();
  int g = server.arg("g").toInt();
  int b = server.arg("b").toInt();
  int h = server.arg("h").toInt();
  
  MatrixPost(x, y, r, g, b, h > 0);
}

void handleIntegerPost() {
  counters[2]++;
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  IntegerPost(pin, value);
}

void handleAnalogWritePost() {
  counters[2]++;
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  AnalogWritePost(pin, value);
}

void handleDigitalWritePost() {
  counters[2]++;
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();
  DigitalWritePost(pin, value);
}

void handleServoWritePost() {
  counters[2]++;
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  ServoWritePost(pin, value);
}

void handleAnalogReadPost() {
  counters[2]++;
  AnalogReadPost();
}

void handleToneWritePost() {
  counters[2]++;
  String pinStr = "";
  pinStr = server.arg("pin");
  int pin = pinStr.toInt();
  //ClearServo(pin);

  String valueStr = server.arg("value");
  unsigned int value = atol(valueStr.c_str());
  ToneWritePost(pin, value);
}

void handleSweepPost() {
  counters[2]++;
  int servoPin = intArg("servo");
  int pwmPin = intArg("pwm");
  int value = intArg("value");
  int low = intArg("low");
  int high = intArg("high");
  int count = intArg("count");
  int speed = intArg("delay");

  pinMode(pwmPin, OUTPUT);
  analogWrite(pwmPin, 0);

  // if (servoPin != servo_pin) {
  //   if (servo_pin != 0) {
  //     //servo_ctrl.detach(servo_pin);
  //   }
  //   //servo_ctrl.attach(servoPin);
  //   servo_pin = servoPin;
  // }

  int pos = low;
  //servo_ctrl.write(pos);
  delay(2500);


  for (int i = 1; i <= count; i++) {

    for (pos; pos <= high; pos++) {
      analogWrite(pwmPin, value);
      //delayMicroseconds(speed);
      //servo_ctrl.write(pos);
      delayMicroseconds(speed);
      analogWrite(pwmPin, 0);
    }
    
    analogWrite(pwmPin, 0);
    delay(100);
    pos = low;
    //servo_ctrl.write(pos);
    delay(1000);


    // for (pos; pos >= low; pos--) {
    //   analogWrite(pwmPin, value);
    //   delayMicroseconds(speed);
    //   servo_ctrl.write(pos);
    //   delayMicroseconds(speed);
    //   analogWrite(pwmPin, 0);
    // }
  }

  analogWrite(pwmPin, 0);

  String response = "{";
  response += jsonField("servoPin", String(servoPin), true);
  response += jsonField("pwmPin", String(pwmPin), true);
  response += jsonField("value", String(value), true);
  response += jsonField("low", String(low), true);
  response += jsonField("high", String(high), true);
  response += jsonField("count", String(count), true);
  response += jsonField("delay", String(speed), false);

  response += "}";

  server.send(200, "application/json", response);
}

void handlePulsePost() {
  counters[2]++;
  String pinStr = "";
  pinStr = server.arg("pin");
  int pin = pinStr.toInt();

  String valueStr = "";
  valueStr = server.arg("value");
  int value = valueStr.toInt();

  String timeStr = "";
  timeStr = server.arg("time");
  int time = timeStr.toInt();

  PulsePost(pin, value, time);
}

void handleApiPost() {
  counters[2]++;
  String cmd = server.arg("cmd");
  cmd.toLowerCase();
  Serial.print("CMD:");
  Serial.println(cmd);

  doBlink = true;
  
  String first_word = str_split(cmd, 0);

  if (first_word == "an" || first_word == "analog") {
    int pin = str_int(cmd, 1);
    int val = str_int(cmd, 2);
    AnalogWritePost(pin,val);
    return;
  }

  // Digital Output (pin, value)
  if (first_word == "di" || first_word == "digital") {
    int pin = str_int(cmd, 1);
    int val = str_int(cmd, 2);
    DigitalWritePost(pin,val);
    return;
  }

  // Tone Output (pin, value)
  if (first_word == "to" || first_word == "tone") {
    int pin = str_int(cmd, 1);
    int val = str_int(cmd, 2);
    ToneWritePost(pin,val);
    return;
  }

  bool response_handled = OnApiCommand(cmd);
  if (!response_handled) {
    send_rec(cmd);
  }
}

void handleDataPost() {
  PrintCore("handleDataPost");

  String src = server.arg("src");
  String type = server.arg("type");
  String var = server.arg("var");
  String val = server.arg("val");
  Serial.println(src);
  Serial.println(type);
  Serial.println(var);
  Serial.println(val);
  

  send_rec("ok");
}

#pragma endregion Post_Handlers

#pragma region Setup

void SetupServer() {
  PrintCore("SetupServer");
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.enableCORS(true);
  server.enableCrossOrigin(true);

  #ifdef ESP32_SITE_H
    server.on("/", HTTP_GET, handleGetMatrix);
    server.on("/ctrl", HTTP_GET, handleGetIndex);
    server.on("/index.min.js", HTTP_GET, handleGetJavascript);
    server.on("/index.min.css", HTTP_GET, handleGetStylesheet);
  #endif

  server.on("/id", HTTP_GET, handleGetId);

  server.on("/mat", HTTP_POST, handleMatrixPost);
  server.on("/integer", HTTP_POST, handleIntegerPost);
  server.on("/analogout", HTTP_POST, handleAnalogWritePost);
  server.on("/digitalout", HTTP_POST, handleDigitalWritePost);
  server.on("/servo", HTTP_POST, handleServoWritePost);
  server.on("/analogin", HTTP_POST, handleAnalogReadPost);
  server.on("/tone", HTTP_POST, handleToneWritePost);
  server.on("/api", HTTP_POST, handleApiPost);
  server.on("/data", HTTP_POST, handleDataPost);

  server.on("/pulse", HTTP_POST, handlePulsePost);
  server.on("/sweep", HTTP_POST, handleSweepPost);
  

  server.onNotFound(handleNotFound);
  server.begin();
}

void SetupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.print("Connected to ");
  Serial.println(ssid);

  delay(1000);
  IPADDRESS = WiFi.localIP().toString() + ":" + String(HTTP_PORT);
  MACADDRESS = WiFi.macAddress();
  Serial.println("URL: http://" + IPADDRESS);
}

void PreSetup();
void SetupPins();
void SetupTimers();
void NetReady();

void Core0Processor(void *parameter) {
  SetupWifi();
  SetupServer();
  NetReady();

  for (;;) {
    server.handleClient();
  }
}

void setup(void) {
  Serial.begin(SERIAL_BAUDRATE);
  delay(1000);

  PrintCore("setup");
  analogReadResolution(12);
  analogWriteResolution(23, 12);

  PreSetup();
  SetupPins();
  SetupTimers();

  xTaskCreatePinnedToCore(
    Core0Processor,   /* Function to implement the task */
    "Core0Processor", /* Name of the task */
    10000,            /* Stack size in words */
    NULL,             /* Task input parameter */
    0,                /* Priority of the task */
    &Task1,           /* Task handle. */
    0);               /* Core where the task should run */

}

#pragma endregion Setup

#endif