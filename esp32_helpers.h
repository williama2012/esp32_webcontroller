#ifndef ESP32_HELPERS
#define ESP32_HELPERS_H
#include <Arduino.h>

#define BLINK_DELAY 50
#define MAX_PIN 64
#define MAX_PARAM 16

typedef struct {
  char *name;
  char *value;
} Aarg;

enum PinModeEnum {
  AnalogWrite = 1,
  AnalogRead = 2,
  DigitalWrite = 3,
  DigitalRead = 4,
  Servo = 5,
  Tone = 6,
  Pulse = 7,
  Integer = 8
};

typedef struct {
  uint8_t pin;
  int value;
  PinModeEnum mode;
} PinSet;


String str_split(String str, uint8_t position, char separator = ' ') {
  String strs[MAX_PARAM];
  int StringCount = 0;

  while (str.length() > 0) {
    int index = str.indexOf(separator);
    if (index == -1) {
      strs[StringCount++] = str;
      break;
    } else {
      strs[StringCount++] = str.substring(0, index);
      str = str.substring(index + 1);
    }
  }

  if (position > StringCount) {
    return "";
  }

  return strs[position];
}

int str_int(String str, uint8_t position, char separator = ' ') {
  String valueStr = str_split(str, position, separator);
  if (valueStr == "") {
    return -1;
  }
  return valueStr.toInt();
}



#pragma region Pins

PinSet PIN_SET[MAX_PIN];

void set_pin(uint8_t pin, PinModeEnum mode, int value) {
  if (pin > MAX_PIN) {
    pin = MAX_PIN;
  }
  PIN_SET[pin].pin = pin;
  PIN_SET[pin].mode = mode;
  PIN_SET[pin].value = value;

  // Aarg args;
  // args.name = "";
  // args.value = "";
  // PIN_SET[pin].args = args;
}

PinSet get_pin(uint8_t pin) {
  if (pin > MAX_PIN) {
    pin = MAX_PIN;
  }
  return PIN_SET[pin];
}

void ResetPins() {
  for (int i = 2; i <= 24; i++) {
    pinMode(i, OUTPUT);
    analogWrite(i, 0);
    pinMode(i, INPUT);
    int val = analogRead(i);
  }
}

#pragma endregion Pins

#pragma region Json

String jsonField(String field, String value, bool addMore = false) {
  return "\"" + field + "\":" + "\"" + value + "\"" + (addMore ? "," : "");
}

String makeStatusItem(int pin, String message, bool includeComma = true) {
  String msg = "{" + jsonField("pin", String(pin), true) + jsonField("status", message, false) + "}";
  if (includeComma) {
    msg += ",";
  }
  return msg;
}

#pragma endregion Json

#pragma region Indicator

void Indicator(bool on) {
  digitalWrite(2, on);
}

void Blink() {
  Indicator(true);
  delay(BLINK_DELAY);
  Indicator(false);
}

#pragma endregion Indicator

#pragma region Printing

void s_print(const String &msg) {
  if (!Serial) { return; }
  Serial.print(msg);
}

void s_print(const char *msg) {
  if (!Serial) { return; }
  Serial.print(F(msg));
}

void s_print(char *msg) {
  if (!Serial) { return; }
  Serial.print(F(msg));
}

void s_print(uint32_t msg) {
  if (!Serial) { return; }
  Serial.print(msg);
}

void s_println(const String &msg) {
  if (!Serial) { return; }
  Serial.println(msg);
}

void s_println(const char *msg) {
  if (!Serial) { return; }
  Serial.println(F(msg));
}

void s_println(char *msg) {
  if (!Serial) { return; }
  Serial.println(F(msg));
}

void s_println(uint32_t msg) {
  if (!Serial) { return; }
  Serial.println(msg);
}

void PrintCore(char *msg) {
  if (!Serial) { return ; }
  s_print("--- " + String(msg) + " running on core ");
  s_print(xPortGetCoreID());
  s_println(" ---");
}

void PrintCore(const String &msg) {
  if (!Serial) { return ; }
  s_print("--- " + String(msg) + " running on core ");
  s_print(xPortGetCoreID());
  s_println(" ---");
}

#pragma endregion Printing

byte *scan_i2c() {
  byte error, address;
  int nDevices = 0;

  byte *array = (byte *)malloc(0x7f * sizeof(byte));

  Serial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {  //0x7f
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      array[nDevices] = address;
      nDevices++;
    } else if (error != 2) {
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }

  return array;
}

#endif