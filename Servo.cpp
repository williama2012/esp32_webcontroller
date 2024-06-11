#include "Servo.h"

/**
 * @brief   attaches the given pin, channel, freq, resolution
 * @param   @pin : servo pin
 *          @channel : channel of pwm
 *          @freq : frequency of pwm
 *          @resolution : range is 1-14 bits (1-20 bits for ESP32)
 * @retval  None
**/

void Servo::attach(int pin, int channel, int freq, int resolution, int servoMax) {
  _channel = channel;
  if(channel > 15) channel = 15;
  _servomax = servoMax;
  ledcSetup(_channel, freq, resolution);
  ledcAttachPin(pin, channel);
  ledcWrite(_channel, FIRSTDUTY);
}

/**
 * @brief   detached the given pin
 * @param   @pin : servo pin
 * @retval  None
**/

void Servo::detach(uint8_t pin) {
  ledcDetachPin(pin);
}


/**
 * @brief   writes servo value 0-_servomax as degree
 * @param   @value: servo value 0-_servomax as degree
 * @retval  None
**/

void Servo::write(int value) {
  if(value < 0) value = 0;
  if(value > _servomax) value = _servomax;
  int servoValue = (value - SERVOMIN) * (DUTYCYLEMAX - DUTYCYLEMIN) / (_servomax - SERVOMIN) + DUTYCYLEMIN; // mapping to SERVOMIN-_servomax values from DUTYCYLEMIN-DUTYCYLEMAX values
  ledcWrite(_channel, servoValue);

}

void Servo::writeMicroseconds(int value) {
  if (value < 0) value = 0;
  if (value > 3000) value = 3000;
  value = map(value, 0, 3000, 0, 180);
  this->write(value);
}

void Servo::setServoMax(int value) {
  if(value <= 0) {
    value = SERVOMAX;
  }
  _servomax = value;
}
