/** 
    Created By: William Allison
    Created Date: Feb 2024
    Description:
    
*/

#ifndef ESP32_TIMER_H
#define ESP32_TIMER_H
#include <Arduino.h>

void AddTimer(String name, unsigned long delay);
unsigned long GetTimer(int timerId);
int GetTimerIndex(String name);
void SetTimer(int timerId, unsigned long delay, String name = "");
bool CheckTimer(int timerId);

#endif