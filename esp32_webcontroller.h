#ifndef ESP32_CONTROLLER
#define ESP32_CONTROLLER_H
#include <Arduino.h>




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

String jsonField(String field, String value, bool addMore);
String makeStatusItem(int pin, String message, bool includeComma);

#endif