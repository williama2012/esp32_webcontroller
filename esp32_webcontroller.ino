#include "esp32_webcontroller.h"
#include "esp32_ledstrip.h"

Timer timers;
uint8_t mode = 1;

void SetupPins() {
  pinMode(23, OUTPUT);
}

void SetupTimers() {
  timers.AddTimer(0, 1);
  timers.AddTimer(1, 10000);

  BeginStrip();
}

PinSet prev_mode;

void loop(void) {

  if (doBlink) {
    Blink();
    doBlink = false;
  }

  //CheckApiCommand();

  PinSet mode = get_pin(50);

  bool mode_changed = mode.value != prev_mode.value;

  if (mode_changed) {
    PrintCore("Mode Changed: " + String(mode.value));
  }

  switch (mode.value) {
    case 1:
      mode1process();
      break;
    case 2:
      mode2process();
      break;
    case 3:
      mode3process();
      break;
    default:
      break;
  }

  prev_mode = mode;
}

PinSet prev_brightness;

void mode3process() {
  PinSet brightness = get_pin(54);
  if (brightness.value != prev_brightness.value) {
    set_brightness(brightness.value);
    prev_brightness = brightness;
  }

  PinSet red_set = get_pin(51);
  PinSet green_set = get_pin(52);
  PinSet blue_set = get_pin(53);
  led_clear();
  setAllColorSequence(red_set.value, green_set.value, blue_set.value);
  
}


void mode2process() {
  PinSet brightness = get_pin(54);
  if (brightness.value != prev_brightness.value) {
    set_brightness(brightness.value);
    prev_brightness = brightness;
  }

  PinSet red_set = get_pin(51);
  PinSet green_set = get_pin(52);
  PinSet blue_set = get_pin(53);
  setAllColor(red_set.value, green_set.value, blue_set.value);
  
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


bool processCmd_setPixel(String cmd) {
    int pixel = str_int(cmd, 1);
    Serial.println(pixel);

    if (pixel < 0) {
      return send_500("format: pixel {pixel#} {color}");
    }
    String color_str = str_split(cmd, 2);
    if (color_str == "") {

      set_pixel(pixel, CRGB(255, 255, 255));

      return send_msg("pixel " + String (pixel) + " set to WHITE");
    }
    CRGB color = led_color(color_str);
    set_pixel(pixel, color);
    return send_msg("pixel " + String (pixel) + " set to " + color_str);
}

// Runs on Core 1
bool OnApiCommand(String cmd) {
  PrintCore("ProcessCommand: " + cmd);
  String first_word = str_split(cmd, 0);
  
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
      set_pin(51, Integer, str_int(cmd, 1));
      set_pin(52, Integer, str_int(cmd, 2));
      set_pin(53, Integer, str_int(cmd, 3));
  }

  if (first_word == "mode") {
    int m = str_int(cmd, 1);
    if (m > -1) {
      set_pin(50, Integer, m);
      return send_msg("mode changed to " + String(m));
    }
    return send_msg("invalid mode value");
  }

  if (first_word == "clear") {
    led_clear();
    return send_msg("cleared");
  }
  
  if (first_word == "pixel") {
    return processCmd_setPixel(cmd);
  }

  return false;
}
