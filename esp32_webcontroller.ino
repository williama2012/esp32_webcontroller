#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "incbin.h"
#include "Servo.h"

INCTXT(WebPage, "index.html");
INCTXT(WebJavascript, "index.js");
INCTXT(WebStylesheet, "index.css");

const char *ssid = "BDS_HOME_24";
const char *password = "bdsWins999";
TaskHandle_t Task1;
TaskHandle_t Task2;

String url;
bool doBlink = false;

WebServer server(80);

int servo_pin = 0;
Servo servo_ctrl;

typedef struct {
  int pin;
  int pos;
  bool isNew;
} servoPosDef;

servoPosDef servoPos;

#pragma region Display

void Blink() {
  PrintCore("Blink");
  Green(false);
  delay(25);
  Green(true);
}

void Red(bool on) {
  digitalWrite(14, !on);
}

void Green(bool on) {
  digitalWrite(15, !on);
}

void Blue(bool on) {
  digitalWrite(16, !on);
}

#pragma endregion Display

#pragma region Printing

String jsonField(String field, String value, bool addMore) {
  return "\"" + field + "\":" + "\"" + value + "\"" + (addMore ? "," : "");
}

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
  int pos = map(value, 0, 4095, 0, 180);

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

    if(pin > 0) {
      ClearServo(pin);
      pinMode(pin, INPUT);
      int value = analogRead(pin);
      response += "{" 
        + jsonField("pin", String(pin), true) 
        + jsonField("value", String(value), false) 
        + "}";
      if(i > 0) {
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

  if(value == 0) {
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

String makeStatusItem(int pin, String message, bool includeComma = true) {
  String msg = "{" + jsonField("pin", String(pin), true) + jsonField("status", message, false) + "}";
  if(includeComma) {
    msg += ",";
  }
  return msg;
}

void handleApiPost() {
  PrintCore("handleApiPost");
  String cmd = server.arg("cmd");
  cmd.toLowerCase();

  if(cmd == "reset") {
    for(int i = 2; i <= 24; i++) {
      servo_ctrl.detach(i);
      pinMode(i, OUTPUT);
      analogWrite(i, 0);
      pinMode(i, INPUT);
      analogRead(i);
      Println("reset pin - " + String(i));
    }

  }

  servo_pin = 0;

  doBlink = true;
  server.send(200, "application/json", "{" + jsonField("reset", "complete", false) + "}");
}

#pragma endregion Post_Handlers

#pragma region Setup

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

void SetupServer() {
  PrintCore("SetupServer");
  if (MDNS.begin("esp32")) {
    Println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleGetIndex);
  server.on("/index.js", HTTP_GET, handleGetJavascript);
  server.on("/index.css", HTTP_GET, handleGetStylesheet);

  server.on("/analogout", HTTP_POST, handleAnalogWritePost);
  server.on("/servo", HTTP_POST, handleServoWritePost);
  server.on("/analogin", HTTP_POST, handleAnalogReadPost);
  server.on("/tone", HTTP_POST, handleToneWritePost);
  server.on("/api", HTTP_POST, handleApiPost);

  server.onNotFound(handleNotFound);
  server.begin();

  Println("HTTP server started");
}

void setup(void) {
  Red(true);
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(115200);

  delay(1000);

  PrintCore("setup");

  Red(false);
  Blue(true);

  SetupWifi();
  SetupServer();

  Blue(false);
  Green(true);

  xTaskCreatePinnedToCore(
    MonitorWebServer,   /* Function to implement the task */
    "MonitorWebServer", /* Name of the task */
    10000,              /* Stack size in words */
    NULL,               /* Task input parameter */
    0,                  /* Priority of the task */
    &Task1,             /* Task handle. */
    0);                 /* Core where the task should run */
}

#pragma endregion Setup

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

#pragma region core loops

void MonitorWebServer(void *parameter) {
  PrintCore("MonitorWebServer");

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
  delay(1);
}

#pragma endregion core loops