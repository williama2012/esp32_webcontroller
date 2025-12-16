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
      break;
    default:
      mode1process();
  }

  prev_mode = mode;
}

void set_pixel(uint8_t x, uint8_t y, uint32_t color) {
  
}

void mode3process() {

}

PinSet prev_brightness;

void mode2process() {
  PinSet brightness = get_pin(54);
  if (brightness.value != prev_brightness.value) {
    strip.setBrightness(brightness.value);
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
      setAllColor(0, color_weight, 0);
    }
  }
}


// Runs on Core 0
bool OnApiCommand(String cmd) {
  PrintCore("OnApiCommand: " + cmd);
  return ProcessCommand(cmd);
}

// Runs on Core 1
bool ProcessCommand(String cmd) {
  PrintCore("ProcessCommand: " + cmd);
  String first_word = str_split(cmd, 0);
  
  if (first_word == "mode") {
    int m = str_int(cmd, 1);
    if (m > -1) {
      set_pin(50, Integer, m);
      return send_msg("mode changed to " + String(mode));
    }
    return send_msg("invalid mode value");
  }

  if (first_word == "clear") {
    setAllColor(BLACK);
    return send_msg("LED Strip Cleared");
  }
  
  if (first_word == "pixel") {
    String pixel_str = str_split(cmd, 1);

    if (pixel_str != "") {
      int pixel = pixel_str.toInt();
      String color_str = str_split(cmd, 2);

      if (color_str != "") {
        int color = color_str.toInt();
        set_pixel(pixel, color);
      } else {
        set_pixel(pixel, WHITE);
      }

      return send_msg("Pixel: " + String(pixel) + " changed to " + String(color_str));
    }

  }

  return false;
}

// Runs on Core 1
void CheckApiCommand() {
  if (api_cmd != "") {
    PrintCore("CheckApiCommand: " + api_cmd);
    bool command_processed = ProcessCommand(api_cmd);
    api_cmd = "";
  }
}
