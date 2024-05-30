#ifndef ESP32_CONTROLLER
#define ESP32_CONTROLLER_H
#include <Arduino.h>

typedef struct {
  int pin;
  int pos;
  bool isNew;
} servoPosDef;

String jsonField(String field, String value, bool addMore);
String makeStatusItem(int pin, String message, bool includeComma);

#endif