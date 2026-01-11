#include "esp32_webcontroller.h"

Timer timers;
uint8_t mode = 0;

void SetupPins() {
  pinMode(23, OUTPUT);
}

void SetupTimers() {
  timers.AddTimer(0, 1);
  timers.AddTimer(1, 10000);

  BeginStrip();
}

// Runs on Core 1
void loop(void) {

  if (doBlink) {
    Blink();
    doBlink = false;
  }

  switch (mode) {
    case 1:
      mode1process();
      break;
    case 3:
      mode3process();
      break;
    default:
      break;
  }

}

void mode3process() {
  led_clear();
  setAllColorSequence(color_r, color_g, color_b);
}

void mode1process() {
  if (timers.CheckTimer(0)) {
    step_weight();

    if (flip == 1) {
      color_r = color_weight;
      color_g = 0;
      color_b = 0;
      setAllColor(color_r, color_g, color_b);
    } else {
      color_r = 0;
      color_g = color_weight;
      color_b = 0;
      setAllColor(color_r, color_g, color_b);
    }
  }
}

bool OnSetParameter(String cmd) {
  PrintCore("OnSetParameter: " + cmd);
  
  String param = str_split(cmd, 1);
  String value = str_split(cmd, 2);

  return send_body(jsonField("param", String(param), true) + jsonField("value", String(value)));
}

// Runs on Core 0
bool OnApiCommand(String cmd) {
  PrintCore("ProcessCommand: " + cmd);
  String first_word = str_split(cmd, 0);
  
  if (first_word == "set") {
    return OnSetParameter(cmd);
  }

  // Analog Output (pin, value)
  if (first_word == "an" || first_word == "analog") {
    int pin = str_int(cmd, 1);
    int val = str_int(cmd, 2);
    AnalogWritePost(pin,val);
    return true;
  }

  // Digital Output (pin, value)
  if (first_word == "di" || first_word == "digital") {
    int pin = str_int(cmd, 1);
    int val = str_int(cmd, 2);
    DigitalWritePost(pin,val);
    return true;
  }

  // Tone Output (pin, value)
  if (first_word == "to" || first_word == "tone") {
    int pin = str_int(cmd, 1);
    int val = str_int(cmd, 2);
    ToneWritePost(pin,val);
    return true;
  }



  if (first_word == "p") {
    int x = str_int(cmd, 1);
    int y = str_int(cmd, 2);
    String color_str = str_split(cmd, 3);
    if (color_str != "") {
      CRGB color = led_color(color_str);
      set_pixel(x, y, color);
      return send_body(jsonField("x", String(x), true) + jsonField("y", String(y), true) + jsonField("color", color_str, false));
    } else {
      set_pixel(x, y);
      return send_body(jsonField("x", String(x), true) + jsonField("y", String(y), true) + jsonField("color", "white", false));
    }
  }

  if (first_word == "color") {
      color_r = str_int(cmd, 1);
      color_g = str_int(cmd, 2);
      color_b = str_int(cmd, 3);
      uint16_t brightness = str_int(cmd, 4);
      if (brightness > -1) {
        set_brightness(brightness);
      }
  }

  if (first_word == "brightness") {
      uint16_t brightness = str_int(cmd, 1);
      if (brightness > -1) {
        set_brightness(brightness);
        return send_body(jsonField("brightness", String(brightness), false));
      }
  }

  if (first_word == "mode") {
    int m = str_int(cmd, 1);
    if (m > -1) {
      mode = m;
      return send_body(jsonField("mode", String(mode), false));
    }
    return send_msg("invalid mode value");
  }

  if (first_word == "clear") {
    led_clear();
    return send_msg("cleared");
  }


  return false;
}
