#include "esp32_webcontroller.h"
Timer timers;

void SetupPins() {
  Println("SetupPins");
  pinMode(2, OUTPUT);
}

void SetupTimers() {
  Println("SetupTimers");

  timers.AddTimer(0, 3000);
  //Blue(true);
}

#pragma region servo

// void MoveServo(int pin, int pos) {
//   PrintCore("MoveServo");

//   if (pin != servo_pin) {
//     if (servo_pin != 0) {
//       servo_ctrl.detach(servo_pin);
//     }
//     servo_ctrl.attach(pin);
//     servo_pin = pin;
//   }

//   servo_ctrl.write(pos);
// }

// void ClearServo(int pin) {
//   if (servo_pin == pin) {
//     servo_ctrl.detach(servo_pin);
//     servo_pin = 0;
//   }
// }

#pragma endregion servo

#pragma region core loops

void Core0Processor(void *parameter) {
  PrintCore("Core0Processor");

  for (;;) {
    server.handleClient();
  }
}

void loop(void) {

  if (doBlink) {
    //Blink();
    doBlink = false;
  }

  if (timers.CheckTimer(0)) {
    Blink();
    //LcdUpdateRows();
  }

  delayMicroseconds(1);
}

#pragma endregion core loops