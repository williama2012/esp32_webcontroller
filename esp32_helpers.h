#ifndef ESP32_HELPERS
#define ESP32_HELPERS_H

#include <Arduino.h>


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