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

void str_pad(String& str, uint8_t length) {
  uint8_t diff = length - str.length();
  
  for(int i = 0; i < diff; i++) {
    str += " ";
  }
}

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

bool check_i2c(byte address) {
  byte error;
  Wire.beginTransmission(address);
  delay(1);
  error = Wire.endTransmission();
  if (error == 0) {
    return true;
  }
  return false;
}

byte *scan_i2c() {
  byte error, address;
  int nDevices = 0;

  byte *array = (byte *)malloc(0x7f * sizeof(byte));

  Serial.println(F("Scanning for I2C devices ..."));
  for (address = 0x01; address < 0x7f; address++) {  //0x7f
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf(F("I2C device found at address 0x%02X\n"), address);
      array[nDevices] = address;
      nDevices++;
    } else if (error != 2) {
      Serial.printf(F("Error %d at address 0x%02X\n"), error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println(F("No I2C devices found"));
  }

  return array;
}

#endif