#ifndef _SERVO_H
#define _SERVO_H

#include <Arduino.h>

#define DELAYMS 2000

#define PWMFREQ 50
#define PWMCHANNEL 0
#define PWMRESOLUTION 12
#define FIRSTDUTY 0

#define SERVOMIN 0
#define SERVOMAX 270
#define DUTYCYLEMIN 100
#define DUTYCYLEMAX 500

class Servo {
public:
  void attach(int pin, int channel = PWMCHANNEL, int freq = PWMFREQ, int resolution = PWMRESOLUTION, int servoMax = SERVOMAX);
  void detach(uint8_t pin);
  void write(int value);
  void writeMicroseconds(int value);
  void setServoMax(int value);
private:
  int _channel;
  int _servomax;
};

#endif
