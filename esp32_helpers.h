#ifndef ESP32_HELPERS
#define ESP32_HELPERS_H
#include <Arduino.h>

#define BLINK_DELAY 50
#define MAX_PIN 64

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

String str_split(String str, uint8_t position) {
  String strs[32];
  int StringCount = 0;

  // Split the string into substrings
  while (str.length() > 0) {
    int index = str.indexOf(' ');
    if (index == -1) { // No space found, last part
      strs[StringCount++] = str;
      break;
    } else {
      strs[StringCount++] = str.substring(0, index);
      str = str.substring(index + 1);
    }
  }

  if(position > StringCount) {
    return "";
  }
  
  return strs[position];
}

int str_int(String str, uint8_t position) {
  String valueStr = str_split(str, position);
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

#pragma endregion Pins

#pragma region Json

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

void PrintCore(String msg) {
  Print("--- " + String(msg) + " running on core ");
  Print(xPortGetCoreID());
  Println(" ---");
}

#pragma endregion Printing




#endif