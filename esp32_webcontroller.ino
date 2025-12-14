#include "esp32_webcontroller.h"

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
    Blink();
    doBlink = false;
  }

  if (CheckTimer(0)) {
    //LcdUpdateRows();
  }

  delayMicroseconds(1);
}

#pragma endregion core loops