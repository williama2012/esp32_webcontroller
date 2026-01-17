/** 
    Created By: William Allison
    Created Date: Feb 2024
    Description:
    
*/

#ifndef ESP32_TIMER_H
#define ESP32_TIMER_H
#include <Arduino.h>

#define MAX_TIMERS 32

class Timer {
public:
  void AddTimer(unsigned int timerId, unsigned long delay);
  unsigned long GetTimer(unsigned int timerId);
  void SetTimer(unsigned int timerId, unsigned long delay);
  bool CheckTimer(unsigned int timerId);
  bool CheckTimer(unsigned int timerId, unsigned long currentMillis);
private:
  unsigned long TIMERS[MAX_TIMERS];
  unsigned long TIMERS_DELAY[MAX_TIMERS];
};

#endif