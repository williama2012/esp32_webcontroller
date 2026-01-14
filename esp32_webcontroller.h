#ifndef ESP32_CONTROLLER
#define ESP32_CONTROLLER_H
#include <Arduino.h>
#include "esp32_server.h"
#include "esp32_timer.h"
#include "esp32_ledstrip.h"
#include "esp32_lcd.h"

#define SERIAL_BAUDRATE 115200
#define VERSION 20260111.01

Timer timers;
TaskHandle_t Task1;

bool doBlink = false;
String IPADDRESS;

bool show_actions = true;
uint32_t action_count = 0;

bool OnApiCommand(String cmd);

#pragma region Actions

void MatrixPost(uint16_t x, uint16_t y, uint16_t r = 255, uint16_t g = 255, uint16_t b = 255) {
  PrintCore("MatrixPost");
  action_count++;

  doBlink = true;

  set_pixel(x, y, CRGB(r, g, b));

  send_body(
    jsonField("x", String(x), true)
    + jsonField("y", String(y), true)
    + jsonField("r", String(r), true)
    + jsonField("g", String(g), true)
    + jsonField("b", String(b), false));
}

void IntegerPost(int pin, int value) {
  PrintCore("IntegerPost");
  action_count++;

  set_pin(pin, Integer, value);

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), false);
  response += "}";
  doBlink = true;

  server.send(200, "application/json", response);
}

void AnalogWritePost(int pin, int value) {
  PrintCore("AnalogWritePost");
  action_count++;

  //ClearServo(pin);
  set_pin(pin, AnalogWrite, value);

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
  PrintCore("DigitalWritePost");
  action_count++;

  //ClearServo(pin);
  set_pin(pin, DigitalWrite, value);

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
  PrintCore("ServoWritePost");
  action_count++;

  int pos = value;
  //int pos = map(value, 0, 4095, 0, 180);

  set_pin(pin, Servo, value);

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
  PrintCore("AnalogReadPost");

  String pinStr = server.arg("pins");
  pinStr.replace("[", "");
  pinStr.replace("]", "");

  Println("pins:" + pinStr);

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
      set_pin(pin, AnalogRead, value);

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
  PrintCore("ToneWritePost");
  action_count++;

  set_pin(pin, Tone, value);

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
  action_count++;
  
  set_pin(pin, Pulse, value);
  //ClearServo(pin);

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

void handleGetIndex() {
  PrintCore("handleGetIndex");
  server.send(200, "text/html", gWebPageData);
}

void handleGetTerminal() {
  PrintCore("handleGetTerminal");
  server.send(200, "text/html", gTerminalWebPageData);
}

void handleGetMatrix() {
  PrintCore("handleGetTerminal");
  server.send(200, "text/html", gMatrixWebPageData);
}


void handleGetJavascript() {
  PrintCore("handleGetJavascript");
  server.send(200, "text/javascript", gWebJavascriptData);
}

void handleGetStylesheet() {
  PrintCore("handleGetStylesheet");
  server.send(200, "text/css", gWebStylesheetData);
}

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

#pragma endregion Get_Handlers

#pragma region Post_Handlers

void handleMatrixPost() {
  PrintCore("handleMatrixPost");
  int x = server.arg("x").toInt();
  int y = server.arg("y").toInt();

  int r = server.arg("r").toInt();
  int g = server.arg("g").toInt();
  int b = server.arg("b").toInt();
  
  MatrixPost(x, y, r, g, b);
}

void handleIntegerPost() {
  PrintCore("handleIntegerPost");
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  IntegerPost(pin, value);
}

void handleAnalogWritePost() {
  PrintCore("handleAnalogWritePost");
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  AnalogWritePost(pin, value);
}

void handleDigitalWritePost() {
  PrintCore("handleDigitalWritePost");
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();
  DigitalWritePost(pin, value);
}

void handleServoWritePost() {
  PrintCore("handleServoWritePost");
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  ServoWritePost(pin, value);
}

void handleAnalogReadPost() {
  PrintCore("handleAnalogReadPost");
  AnalogReadPost();
}

void handleToneWritePost() {
  PrintCore("handleToneWritePost");
  String pinStr = "";
  pinStr = server.arg("pin");
  int pin = pinStr.toInt();
  //ClearServo(pin);

  String valueStr = server.arg("value");
  unsigned int value = atol(valueStr.c_str());
  ToneWritePost(pin, value);
}

void handleSweepPost() {
  int servoPin = intArg("servo");
  int pwmPin = intArg("pwm");
  int value = intArg("value");
  int low = intArg("low");
  int high = intArg("high");
  int count = intArg("count");
  int speed = intArg("delay");

  pinMode(pwmPin, OUTPUT);
  analogWrite(pwmPin, 0);

  set_pin(servoPin, Servo, count);
  set_pin(pwmPin, AnalogWrite, value);

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
  PrintCore("handlePulsePost");

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

#pragma endregion Post_Handlers

void handleApiPost() {
  PrintCore("handleApiPost");
  String cmd = server.arg("cmd");
  cmd.toLowerCase();
  
  doBlink = true;
  
  String first_word = str_split(cmd, 0);

  if (first_word == "reset") {
    ResetPins();
    server.send(200, "application/json", "{" + jsonField("reset", "complete", false) + "}");
    return;
  }

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

#pragma region Setup

void SetupServer() {
  PrintCore("SetupServer");
  if (MDNS.begin("esp32")) {
    Println("MDNS responder started");
  }
  server.enableCORS(true);
  server.enableCrossOrigin(true);

  // server.on("/", HTTP_GET, handleGetIndex);
  // server.on("/matrix", HTTP_GET, handleGetMatrix);

  server.on("/", HTTP_GET, handleGetMatrix);
  server.on("/ctrl", HTTP_GET, handleGetIndex);
  server.on("/matrix", HTTP_GET, handleGetMatrix);
  server.on("/terminal", HTTP_GET, handleGetTerminal);

  server.on("/index.js", HTTP_GET, handleGetJavascript);
  server.on("/index.css", HTTP_GET, handleGetStylesheet);

  server.on("/mat", HTTP_POST, handleMatrixPost);
  server.on("/color", HTTP_POST, handleIntegerPost);
  server.on("/integer", HTTP_POST, handleIntegerPost);
  server.on("/analogout", HTTP_POST, handleAnalogWritePost);
  server.on("/digitalout", HTTP_POST, handleDigitalWritePost);
  server.on("/servo", HTTP_POST, handleServoWritePost);
  server.on("/analogin", HTTP_POST, handleAnalogReadPost);
  server.on("/tone", HTTP_POST, handleToneWritePost);
  server.on("/api", HTTP_POST, handleApiPost);
  server.on("/pulse", HTTP_POST, handlePulsePost);
  server.on("/sweep", HTTP_POST, handleSweepPost);

  server.onNotFound(handleNotFound);
  server.begin();

  Println("HTTP server started");
  //lcd_clear();
  //lcd_print("HTTP server started");
  //lcd_print(IPADDRESS, 1);
}

void SetupWifi() {
  PrintCore("SetupWifi");

  //lcd_print("SSID:" + String(ssid), 0);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //lcd_print("Connecting..", 1);

  bool o = false;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Print(".");
    o = o ? false : true;
    //lcd_print(o ? "|" : "-", 1, 12);
    //lcd_print("Status:" + String(WiFi.status()), 2);
  }
  //lcd_print("Status:" + String(WiFi.status()), 2);

  Println("");
  Print("Connected to ");
  Println(ssid);
  IPADDRESS = WiFi.localIP().toString() + ":" + String(HTTP_PORT);
  Println("URL: http://" + IPADDRESS);
}

void PreSetup();
void PostSetup();
void SetupPins();
void SetupTimers();
void NetReady();

void Core0Processor(void *parameter) {
  PrintCore("Core0Processor");
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

  PostSetup();
}

#pragma endregion Setup

#endif