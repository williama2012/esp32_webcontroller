/** 
    Created By: William Allison
    Created Date: Feb 2024
    Description:
    
*/

#include "esp32_timer.h"

void Timer::AddTimer(unsigned int timerId, unsigned long delay) {
  TIMERS[timerId] = 0;
  TIMERS_DELAY[timerId] = delay;
}

void Timer::SetTimer(unsigned int timerId, unsigned long delay) {
  TIMERS_DELAY[timerId] = delay;
}

unsigned long Timer::GetTimer(unsigned int timerId) {
  return TIMERS_DELAY[timerId];
}

// Check if time has elapsed for timerId
bool Timer::CheckTimer(unsigned int timerId) {
  unsigned long currentMillis = millis();
  if (currentMillis - TIMERS[timerId] >= GetTimer(timerId)) {
    TIMERS[timerId] = currentMillis;
    return true;
  }
  return false;
}

bool Timer::CheckTimer(unsigned int timerId, unsigned long currentMillis) {
  if (currentMillis - TIMERS[timerId] >= GetTimer(timerId)) {
    TIMERS[timerId] = currentMillis;
    return true;
  }
  return false;
}
