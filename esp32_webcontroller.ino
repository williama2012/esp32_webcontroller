#include "esp32_webcontroller.h"
#include "esp32_net.h"
#include <DHT22.h>
#include <ArduinoJson.h>

#define DHTPIN 23
#define ONE_WIRE_COUNT 1
#define ONE_WIRE_TYPE "dev"
const bool USE_LCD = true;
const bool USE_LED = false;

DHT22 dht22(DHTPIN);

uint8_t mode = 10;
bool show_rssi = true;

void PreSetup() {
  if (USE_LCD) {
    lcd_init();
  }

  ds_init();
}

void PostSetup() {
  if (USE_LCD) {
    lcd_clear();
  }
  if (USE_LED) {
    set_pixel(11, 11, 255, 255, 255);
  }
}

void NetReady() {
  if (USE_LCD) {
    delay(3000);
    lcd_clear();
  }

}

void SetupPins() {

	pinMode(12, OUTPUT); // Trigger
	pinMode(13, INPUT);  // Echo

  pinMode(25, INPUT);  // Microwave

	pinMode(26, OUTPUT); // Trigger
	pinMode(27, INPUT);  // Echo

}

void SetupTimers() {
  timers.AddTimer(0, 3000);
  timers.AddTimer(10, 1000);
  timers.AddTimer(21, 100);
}

float GetRange(uint8_t triggerPin, uint8_t echoPin) {
	digitalWrite(triggerPin, LOW);
	delayMicroseconds(2);
	digitalWrite(triggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(triggerPin, LOW);
  float duration = pulseIn(echoPin, HIGH);
  return (duration * 0.0343)/2;
}

int microwave;

void PollSensors(bool postData = false) {
  float* temps = ds_temps(ONE_WIRE_COUNT);
  for(int i = 0; i < ONE_WIRE_COUNT; i++) {
    float temp = temps[i];

    String txt = "Sensor " + String(i) + ": " + String(temp);
    lcd_print(txt, i);

    if (postData) {
      String response = post_data(IPADDRESS, ONE_WIRE_TYPE, "sensor_" + String(i), temp);
      if (response == NET_FAILED_RESPONSE) {
        lcd_print("X");
      }
    }
  }
}

// Runs on Core 1
void loop(void) {
  //microwave = digitalRead(25);

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
    case 10:
      if (timers.CheckTimer(10)) {
        PollSensors(true);
      }
    case 11:
      if (timers.CheckTimer(10)) {
        PollSensors(false);
      }
    default:
      break;
  }

  if (timers.CheckTimer(0)) {
    if (show_rssi) {
      String rssi = String(WiFi.RSSI()) + " dBm";
      lcd_print_r(rssi, 3);
    }
    if (USE_LCD) {
      lcd_print(String(counter), 3);
    }
    // if (show_temp) {
    //   float temp = dht22.getTemperature(false);
    //   float hum = dht22.getHumidity();
    //   lcd_print(String(temp) + "'F " + String(hum) + "%", 3);
    // }

  }

  // if (timers.CheckTimer(21)) {
  //   lcd_print("D:" + String(GetRange(12, 13)), 0);
  //   delay(10);
  //   lcd_print("D:" + String(GetRange(26, 27)), 1);
  //   lcd_print_r(String(microwave));
  // }
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

bool HandleLcdCommand(String cmd) {
  String cmd_1 = str_split(cmd, 1);
  String cmd_2 = str_split(cmd, 2);

  if (cmd_1 == "init") {
    lcd_init();
    return send_msg("initialized");
  }

  if (cmd_1 == "clear") {
    lcd_clear();
    return send_msg("cleared");
  }

  // backlight
  if (cmd_1 == "bl") {
    if (cmd_2 == "on") {
      lcd_backlight(true);
      return send_msg("backlight on");
    }
    if (cmd_2 == "off") {
      lcd_backlight(false);
      return send_msg("backlight off");
    }
  }

  if (cmd_1 == "print") {
    int row = str_int(cmd, 3);
    int col = str_int(cmd, 4);
    if (row > -1) {
      if(col > -1) {
        lcd_print(cmd_2, row, col);
      } else {
        lcd_print(cmd_2, row);
      }
    } else {
      lcd_print(cmd_2);
    }
    return send_msg("print " + cmd_2);
  }

  if (cmd_1 == "printr") {
    int row = str_int(cmd, 3);
    int col = str_int(cmd, 4);
    if (row > -1) {
      if(col > -1) {
        lcd_print_r(cmd_2, row, col);
      } else {
        lcd_print_r(cmd_2, row);
      }
    } else {
      lcd_print_r(cmd_2);
    }
    return send_msg("print " + cmd_2);
  }


  if (cmd_1 == "show") {
    if (cmd_2 == "ip") {
      lcd_print(IPADDRESS);
    }
    if (cmd_2 == "mac") {
      lcd_print(String(WiFi.macAddress()));
    }
    if (cmd_2 == "version") {
      lcd_print(String(VERSION));
    }
    if (cmd_2 == "rssi") {
      show_rssi = show_rssi ? false : true;
    }

    return send_msg("show " + cmd_2);
  }

  return send_msg("received");
}

bool HandleLedCommand(String cmd) {
  String cmd_1 = str_split(cmd, 1);

  if (cmd_1 == "clear") {
    led_clear();
    return send_msg("cleared");
  }

  if (cmd_1 == "color") {
      String color_str = str_split(cmd, 2);
      int color_r = str_int(color_str, 0, ',');
      int color_g = str_int(color_str, 1, ',');
      int color_b = str_int(color_str, 2, ',');
      int brightness = str_int(color_str, 3, ',');
      if (brightness > -1) {
        set_brightness(brightness);
      }

      setAllColor(color_r, color_g, color_b);

      return send_body(
        jsonField("color_r", String(color_r), true) 
        + jsonField("color_g", String(color_g), true)
        + jsonField("color_b", String(color_b), true)
        + jsonField("brightness", String(brightness), false)
      );
  }

  if (cmd_1 == "set") {
      String pos_str = str_split(cmd, 2);
      int x = str_int(pos_str, 0, ',');
      int y = str_int(pos_str, 1, ',');

      String color_str = str_split(cmd, 3);
      int color_r = str_int(color_str, 0, ',');
      int color_g = str_int(color_str, 1, ',');
      int color_b = str_int(color_str, 2, ',');
      int brightness = str_int(color_str, 3, ',');
      if (brightness > -1) {
        set_brightness(brightness);
      }
      set_pixel(x, y, color_r, color_g, color_b);

      return send_body(
        jsonField("x", String(x), true)
        + jsonField("y", String(y), true)
        + jsonField("color_r", String(color_r), true)
        + jsonField("color_g", String(color_g), true)
        + jsonField("color_b", String(color_b), true)
        + jsonField("brightness", String(brightness), false)
      );
  }

  if (cmd_1 == "seti") {
      int i = str_int(cmd, 2);

      String color_str = str_split(cmd, 3);
      int color_r = str_int(color_str, 0, ',');
      int color_g = str_int(color_str, 1, ',');
      int color_b = str_int(color_str, 2, ',');
      int brightness = str_int(color_str, 3, ',');
      if (brightness > -1) {
        set_brightness(brightness);
      }
      set_pixel_i(i, color_r, color_g, color_b);

      return send_body(
        jsonField("i", String(i), true)
        + jsonField("color_r", String(color_r), true)
        + jsonField("color_g", String(color_g), true)
        + jsonField("color_b", String(color_b), true)
        + jsonField("brightness", String(brightness), false)
      );
  }

  if (cmd_1 == "brightness") {
      uint16_t brightness = str_int(cmd, 2);
      if (brightness > -1) {
        set_brightness(brightness);
        return send_body(jsonField("brightness", String(brightness), false));
      }
  }

  return send_msg("received");
}

// Runs on Core 0
bool OnApiCommand(String cmd) {
  PrintCore("ApiCommand [" + cmd + "]");
  String first_word = str_split(cmd, 0);
  
  //lcd_print(cmd, 2);

  if (first_word == "set") {
    return OnSetParameter(cmd);
  }

  if (first_word == "lcd") {
    return HandleLcdCommand(cmd);
  }

  if (first_word == "led") {
    return HandleLedCommand(cmd);
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

  if (first_word == "mode") {
    int m = str_int(cmd, 1);
    if (m > -1) {
      mode = m;
      return send_body(jsonField("mode", String(mode), false));
    }
    return send_msg("invalid mode value");
  }

  if (first_word == "timer") {
    int timer_id = str_int(cmd, 1);
    int timer_delay = str_int(cmd, 2);
    timers.SetTimer(timer_id, timer_delay);
  }

  /**   */

  if (first_word == "scan_i2c") {
    byte *addresses = scan_i2c();
    uint8_t length = sizeof(addresses);

    s_println(String(sizeof(addresses[0])));
    s_println(String(sizeof(addresses)));
    
    String response = "[";
    // uint8_t i;
    // for (i = 0; i < length; i++) {
    //   byte address = addresses[i];
    //   response += "\"" + String(address, HEX) + "\",";
    // }

    if (response.length() > 1) {
      //response = response.substring(0, response.length() - 2); // remove last comma
    }

    response += "]";

    return send_msg(response);
  }

  if (first_word == "test") {

    JsonDocument doc;

    doc["src"] = MACADDRESS;
    doc["type"] = "sensors";
    doc["var"] = "refridgerator";

    doc["val"][0]["id"] = "sensor0";
    doc["val"][0]["type"] = "temp_sensor";
    doc["val"][0]["var"] = "temp";
    doc["val"][0]["val"] = "75.42";

    doc["val"][1]["id"] = "sensor1";
    doc["val"][1]["type"] = "temp_sensor";
    doc["val"][1]["var"] = "temp";
    doc["val"][1]["val"] = "-1.51";

    doc["val"][2]["id"] = "sensor2";
    doc["val"][2]["type"] = "motion_sensor";
    doc["val"][2]["var"] = "active";
    doc["val"][2]["val"] = "1";

    String buffer;
    serializeJson(doc, buffer);

    String url = DATA_URL;
    url += "?src=" + urlEncode("01:01:01:01:01:01");
    //String response = net_post(url, buffer);

    //String response;
    //post_data(MACADDRESS, "type0", "arg0", "val_0x01", response);

  }

  return false;
}


