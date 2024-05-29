/** 
    Created By: William Allison
    Created Date: Feb 2024
    Description:
    
*/


#include "esp32_timer.h"

const int MAX_TIMERS = 16;
unsigned long TIMERS[MAX_TIMERS];
unsigned long TIMERS_DELAY[MAX_TIMERS];
String TIMERS_NAME[MAX_TIMERS];
int CNT_TIMERS = 0;

void AddTimer(String name, unsigned long delay) {
  TIMERS[CNT_TIMERS] = 0;
  TIMERS_DELAY[CNT_TIMERS] = delay;
  TIMERS_NAME[CNT_TIMERS] = name;
  CNT_TIMERS++;
}

void SetTimer(int timerId, unsigned long delay, String name) {
  TIMERS_DELAY[timerId] = delay;
  if(name.length() > 0) {
    TIMERS_NAME[timerId] = name;
  }
}

// Lookup Index of Timer by Name. Inefficient, avoid using.
int GetTimerIndex(String name) {
  for (int i = 0; i <= MAX_TIMERS; i++) {
    if (TIMERS_NAME[i] == name) {
      return i;
    }
  }
  return -1;
}

unsigned long GetTimer(int timerId) {
  return TIMERS_DELAY[timerId];
}

// Check if time has elapsed for timerId
bool CheckTimer(int timerId) {
  unsigned long currentMillis = millis();
  if (currentMillis - TIMERS[timerId] >= GetTimer(timerId)) {
    TIMERS[timerId] = currentMillis;
    return true;
  }
  return false;
}
