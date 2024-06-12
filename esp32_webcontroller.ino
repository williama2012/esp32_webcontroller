#include "esp32_webcontroller.h"
#include "esp32_timer.h"
#include "secrets.h"
#include "Servo.h"
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
Servo servo_ctrl;
LiquidCrystal_I2C lcd(0x27, 20, 2);

INCTXT(WebPage, "index.html");
INCTXT(WebJavascript, "index.js");
INCTXT(WebStylesheet, "index.css");

String url;
bool doBlink = false;
int servo_pin = 0;
servoPosDef servoPos;

#pragma region server

int intArg(String name) {
  String strVal = server.arg(name);
  int val = strVal.toInt();
  return val;
}

#pragma endregion server


#pragma region lcd

String lcd_row1;
String lcd_row2;
String lcd_row3;
String lcd_row4;

String _lcd_row1_printed;
String _lcd_row2_printed;
String _lcd_row3_printed;
String _lcd_row4_printed;

void LcdUpdateRows() {

  if (lcd_row1 != _lcd_row1_printed
      || lcd_row2 != _lcd_row2_printed
      || lcd_row3 != _lcd_row3_printed
      || lcd_row4 != _lcd_row4_printed) {

    lcd.clear();
    _lcd_row1_printed = "";
    _lcd_row2_printed = "";
    _lcd_row3_printed = "";
    _lcd_row4_printed = "";
  }

  if (lcd_row1 != _lcd_row1_printed) {
    lcd.setCursor(0, 0);
    lcd.print(lcd_row1);
    _lcd_row1_printed = lcd_row1;
  }

  if (lcd_row2 != _lcd_row2_printed) {
    lcd.setCursor(0, 1);
    lcd.print(lcd_row2);
    _lcd_row2_printed = lcd_row2;
  }

  if (lcd_row3 != _lcd_row3_printed) {
    lcd.setCursor(0, 2);
    lcd.print(lcd_row3);
    _lcd_row3_printed = lcd_row3;
  }

  if (lcd_row4 != _lcd_row4_printed) {
    lcd.setCursor(0, 3);
    lcd.print(lcd_row4);
    _lcd_row4_printed = lcd_row4;
  }
}

#pragma endregion lcd

#pragma region led

void Blink() {
  return;

  PrintCore("Blink");

  Red(false);
  Blue(true);
  Green(false);

  delay(25);

  Red(false);
  Blue(false);
  Green(true);
}

void Red(bool on) {
  digitalWrite(14, !on);
}

void Green(bool on) {
  if (on) {
    analogWrite(15, 3900);
  } else {
    digitalWrite(15, 0);
  }

  //digitalWrite(15, !on);
}

void Blue(bool on) {
  digitalWrite(16, !on);
}

#pragma endregion led

#pragma region servo

void MoveServo(int pin, int pos) {
  PrintCore("MoveServo");

  if (pin != servo_pin) {
    if (servo_pin != 0) {
      servo_ctrl.detach(servo_pin);
    }
    servo_ctrl.attach(pin);
    servo_pin = pin;
  }

  servo_ctrl.write(pos);
}

void ClearServo(int pin) {
  if (servo_pin == pin) {
    servo_ctrl.detach(servo_pin);
    servo_pin = 0;
  }
}

#pragma endregion servo

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

#pragma region Get_Handlers

void handleGetIndex() {
  PrintCore("handleGetIndex");
  server.send(200, "text/html", gWebPageData);
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

  ClearServo(pin);

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

  lcd_row2 = "AnalogWrite";
  lcd_row3 = "pin:" + pinStr + ",val:" + valueStr;
  server.send(200, "application/json", response);
}

void handleAnalogWritePost() {
  PrintCore("handleAnalogWritePost");
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  ClearServo(pin);

  pinMode(pin, OUTPUT);
  analogWrite(pin, value);

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), false);

  response += "}";
  doBlink = true;

  lcd_row2 = "AnalogWrite";
  lcd_row3 = "pin:" + pinStr + ",val:" + valueStr;
  server.send(200, "application/json", response);
}

void handleDigitalWritePost() {
  PrintCore("handleDigitalWritePost");
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  ClearServo(pin);

  pinMode(pin, OUTPUT);
  digitalWrite(pin, value);

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), false);

  response += "}";
  doBlink = true;

  lcd_row2 = "AnalogWrite";
  lcd_row3 = "pin:" + pinStr + ",val:" + valueStr;
  server.send(200, "application/json", response);
}


void handleServoWritePost() {
  PrintCore("handleServoWritePost");
  String pinStr = "";
  pinStr = server.arg("pin");
  String valueStr = "";
  valueStr = server.arg("value");
  int pin = pinStr.toInt();
  int value = valueStr.toInt();

  int pos = value;
  //int pos = map(value, 0, 4095, 0, 180);

  MoveServo(pin, pos);
  // servoPos.pin = pin;
  // servoPos.pos = pos;
  // servoPos.isNew = true;

  String response = "{";
  response += jsonField("pin", String(pin), true);
  response += jsonField("value", String(value), true);
  response += jsonField("pos", String(pos), false);
  response += "}";
  doBlink = true;

  lcd_row2 = "ServoWrite";
  lcd_row3 = "pin:" + pinStr + ",val:" + valueStr;
  server.send(200, "application/json", response);
}

void handleAnalogReadPost() {
  PrintCore("handleAnalogReadPost");

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
      ClearServo(pin);
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
  return;
}

void handleToneWritePost() {
  PrintCore("handleToneWritePost");
  String pinStr = "";
  pinStr = server.arg("pin");
  int pin = pinStr.toInt();
  ClearServo(pin);

  String valueStr = server.arg("value");
  unsigned int value = atol(valueStr.c_str());

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
  lcd_row2 = "ToneWrite";
  lcd_row3 = "pin:" + pinStr + ",val:" + valueStr;
  server.send(200, "application/json", response);
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

  if (servoPin != servo_pin) {
    if (servo_pin != 0) {
      servo_ctrl.detach(servo_pin);
    }
    servo_ctrl.attach(servoPin);
    servo_pin = servoPin;
  }

  //analogWrite(pwmPin, 0);

  servo_ctrl.write(low);

  int pos = low;

  analogWrite(pwmPin, value);

  for (int i = 1; i <= count; i++) {

    for (pos; pos <= high; pos++) {
      servo_ctrl.write(pos);
      delay(speed);
    }

    for (pos; pos >= low; pos--) {
      servo_ctrl.write(pos);
      delay(speed);
    }
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

void handleApiPost() {
  PrintCore("handleApiPost");
  String cmd = server.arg("cmd");
  cmd.toLowerCase();

  if (cmd == "reset") {
    for (int i = 2; i <= 24; i++) {
      servo_ctrl.detach(i);
      pinMode(i, OUTPUT);
      analogWrite(i, 0);
      pinMode(i, INPUT);
      analogRead(i);
      Println("reset pin - " + String(i));
    }
    lcd_row2 = "All reset";
    lcd_row3 = "";
  }

  servo_pin = 0;

  doBlink = true;
  server.send(200, "application/json", "{" + jsonField("reset", "complete", false) + "}");
}

#pragma endregion Post_Handlers

#pragma region Setup

void SetupServer() {
  PrintCore("SetupServer");
  if (MDNS.begin("esp32")) {
    Println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleGetIndex);
  server.on("/index.js", HTTP_GET, handleGetJavascript);
  server.on("/index.css", HTTP_GET, handleGetStylesheet);

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
}

void SetupLCD() {
  lcd.init();
  lcd.backlight();
  lcd.noBlink();
  lcd.clear();
}

void SetupTimers() {
  AddTimer("LCD_DISPLAY", 250);
}

void SetupWifi() {
  PrintCore("SetupWifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Print(".");
  }
  Println("");
  Print("Connected to ");
  Println(ssid);
  url = "http://" + WiFi.localIP().toString();
  Println("URL: " + url);
}

void setup(void) {
  Red(true);
  Serial.begin(SERIAL_BAUDRATE);
  delay(1000);

  PrintCore("setup");
  analogReadResolution(12);
  analogWriteResolution(12);

  Red(false);
  Blue(true);

  SetupLCD();
  SetupWifi();
  SetupServer();
  SetupTimers();

  xTaskCreatePinnedToCore(
    Core0Processor,   /* Function to implement the task */
    "Core0Processor", /* Name of the task */
    10000,            /* Stack size in words */
    NULL,             /* Task input parameter */
    0,                /* Priority of the task */
    &Task1,           /* Task handle. */
    0);               /* Core where the task should run */

  lcd_row1 = url;
  Blue(false);
  Green(true);
}

#pragma endregion Setup

#pragma region core loops

void Core0Processor(void *parameter) {
  PrintCore("Core0Processor");

  for (;;) {
    server.handleClient();
  }
}

void loop(void) {
  if (servoPos.isNew == true) {
    MoveServo(servoPos.pin, servoPos.pos);
    servoPos.isNew = false;
  }

  if (doBlink) {
    Blink();
    doBlink = false;
  }

  if (CheckTimer(0)) {
    LcdUpdateRows();
  }

  delayMicroseconds(1);
}

#pragma endregion core loops