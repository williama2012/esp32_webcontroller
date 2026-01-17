#include "esp32_webcontroller.h"

#define ONE_WIRE_COUNT 1
#define ONE_WIRE_TYPE "led_matrix"
uint8_t mode = 10;
bool show_rssi = false;

bool lcd_show_network = true;
bool lcd_show_rssi = true;
bool serial_debug = false;

int Wifi_Signal;
String _net_response;

#pragma region Setup

void SetupTimers() {
  timers.AddTimer(0, 100);  // Network Counts
  timers.AddTimer(1, 3000); // Wifi Signal
  timers.AddTimer(10, 101);
  timers.AddTimer(20, 501);
}

void PreSetup() {
  #ifdef ESP32_LCD_H
    lcd_init();
  #endif
  
  #ifdef ESP32_LED_H
    led_init();
  #endif
  
  #ifdef ESP32_DHT_H
    dht.begin();
  #endif

  #ifdef ESP32_ONEWIRE_H
    ds_init();
  #endif
}

void NetReady() {
  #ifdef ESP32_LED_H
    set_pixel(11, 11, 255, 255, 255);
  #endif

  reset_counters();
}

void SetupPins() {

	pinMode(12, OUTPUT);  // Trigger
	pinMode(13, INPUT);   // Echo

  pinMode(18, INPUT);   // DHT
  pinMode(19, INPUT);   // LED
  pinMode(23, INPUT);   // OneWire
  pinMode(25, INPUT);   // Microwave

	pinMode(26, OUTPUT);  // Trigger
	pinMode(27, INPUT);   // Echo

}
  
#pragma endregion Setup

void send_data(const String& src, const String& type, const String& var, const String& val) {
  int response = post_data(src, type, var, val, _net_response);
  if (serial_debug) {
    Serial.print(F("POST:"));
    Serial.print(type);
    Serial.print(F(", "));
    Serial.print(var);
    Serial.print(F(", "));
    Serial.println(val);
  }

  if (response != HTTP_CODE_OK) {
    #ifdef ESP32_LCD_H
      char str[4];
      sprintf(str, "%d", response);
      lcd_print_r(str);
    #endif

    if (response == -2 || response == -11) {
      #ifdef ESP32_LCD_H
        lcd_print("!! CRIT FAILURE !");
        lcd_print("!! REBOOTING IN 5 !!", 1);
        delay(5000);
        resetFunc();
      #endif

      resetFunc();
    }

    counters[1]++;
  } else {
    counters[0]++;
  }
}

#pragma region Testing

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
float* temps;
float temp;
String PollSensors_txt;

void PollSensors(bool postData = false) {
  #ifdef ESP32_ONEWIRE_H
    temps = ds_temps(ONE_WIRE_COUNT);
    for(int i = 0; i < ONE_WIRE_COUNT; i++) {
      temp = temps[i];

      #ifdef ESP32_LCD_H
        PollSensors_txt = "Sensor_" + String(i) + ": " + String(temp);
        str_pad(PollSensors_txt, 20);
        lcd_print(PollSensors_txt, i);
      #endif

      if (postData) {
        send_data(IPADDRESS, ONE_WIRE_TYPE, "sensor_" + String(i), String(temp));
      }
    }
  #endif
}

#pragma endregion Testing

// Runs on Core 1
void loop(void) {
  //microwave = digitalRead(25);

  if (doBlink) {
    Blink();
    doBlink = false;
  }

  switch (mode) {
    case 1:
      break;
    case 10:
      if (timers.CheckTimer(10)) {
        PollSensors(true);
      }
      break;
    case 11:
      if (timers.CheckTimer(10)) {
        PollSensors(false);
      }
    default:
      break;
  }

  #ifdef ESP32_LCD_H
    if (lcd_show_network && timers.CheckTimer(0)) {
      lcd_print(String(counters[0]) + "|" + String(counters[1]) + "|" + String(counters[2]), 3);
    }
  #endif

  #ifdef ESP32_LCD_H
    if (lcd_show_rssi && timers.CheckTimer(1)) {
      Wifi_Signal = WiFi.RSSI();
      //TODO: Make specific function to not make a new String.
      lcd_print_r((String(Wifi_Signal) + " dBm"), 3);
    }
  #endif

  #ifdef ESP32_DHT_H
    if (timers.CheckTimer(1)) {
        float temp, hum;
        dht_getvalues(temp, hum);

        if (serial_debug) {
          Serial.print(F("Temp:"));
          Serial.print(temp);
          Serial.print(F(" Hum:"));
          Serial.println(hum);
        }
        
        send_data(IPADDRESS, ONE_WIRE_TYPE, "temp", String(temp));
        send_data(IPADDRESS, ONE_WIRE_TYPE, "hum", String(hum));
    }
  #endif
  // if (timers.CheckTimer(21)) {
  //   lcd_print("D:" + String(GetRange(12, 13)), 0);
  //   delay(10);
  //   lcd_print("D:" + String(GetRange(26, 27)), 1);
  //   lcd_print_r(String(microwave));
  // }
}

void mode1process() {
  // if (timers.CheckTimer(0)) {
  //   step_weight();

  //   if (flip == 1) {
  //     color_r = color_weight;
  //     color_g = 0;
  //     color_b = 0;
  //     setAllColor(color_r, color_g, color_b);
  //   } else {
  //     color_r = 0;
  //     color_g = color_weight;
  //     color_b = 0;
  //     setAllColor(color_r, color_g, color_b);
  //   }
  // }
}

#pragma region Handlers

bool HandleSetParameter(String cmd) {
  //PrintCore("OnSetParameter: " + cmd);
  
  String param = str_split(cmd, 1);
  String value = str_split(cmd, 2);

  return send_body(jsonField("param", String(param), true) + jsonField("value", String(value)));
}

bool HandleLcdCommand(String& cmd) {
  String cmd_1 = str_split(cmd, 1);
  String cmd_2 = str_split(cmd, 2);

  if (cmd_1 == "init") {
    #ifdef ESP32_LCD_H
      lcd_init();
    #endif
    return send_msg("initialized");
  }

  if (cmd_1 == "clear") {
    #ifdef ESP32_LCD_H
      lcd_clear();
    #endif
    return send_msg("cleared");
  }

  // backlight
  if (cmd_1 == "bl") {
    if (cmd_2 == "on") {
      #ifdef ESP32_LCD_H
        lcd_backlight(true);
      #endif
      return send_msg("backlight on");
    }
    if (cmd_2 == "off") {
      #ifdef ESP32_LCD_H
        lcd_backlight(false);
      #endif
      return send_msg("backlight off");
    }
  }

  if (cmd_1 == "print") {
    int row = str_int(cmd, 3);
    int col = str_int(cmd, 4);
    if (row > -1) {
      if(col > -1) {
        #ifdef ESP32_LCD_H
          lcd_print(cmd_2, row, col);
        #endif
      } else {
        #ifdef ESP32_LCD_H
          lcd_print(cmd_2, row);
        #endif
      }
    } else {
      #ifdef ESP32_LCD_H
        lcd_print(cmd_2);
      #endif
    }
    return send_msg("print " + cmd_2);
  }

  if (cmd_1 == "printr") {
    int row = str_int(cmd, 3);
    int col = str_int(cmd, 4);
    if (row > -1) {
      if(col > -1) {
        #ifdef ESP32_LCD_H
          lcd_print_r(cmd_2, row, col);
        #endif
      } else {
        #ifdef ESP32_LCD_H
          lcd_print_r(cmd_2, row);
        #endif
      }
    } else {
      #ifdef ESP32_LCD_H
        lcd_print_r(cmd_2);
      #endif
    }
    return send_msg("print " + cmd_2);
  }


  if (cmd_1 == "show") {
    if (cmd_2 == "ip") {
      #ifdef ESP32_LCD_H
        lcd_print(IPADDRESS);
      #endif
    }
    if (cmd_2 == "mac") {
      #ifdef ESP32_LCD_H
        lcd_print(String(WiFi.macAddress()));
      #endif
    }
    if (cmd_2 == "version") {
      #ifdef ESP32_LCD_H
        lcd_print(String(VERSION));
      #endif
    }
    if (cmd_2 == "rssi") {
      show_rssi = show_rssi ? false : true;
    }

    return send_msg("show " + cmd_2);
  }

  return send_msg("received");
}

bool HandleLedCommand(String& cmd) {
  String cmd_1 = str_split(cmd, 1);

  if (cmd_1 == "clear") {
    #ifdef ESP32_LED_H
      led_clear();
    #endif
    return send_msg("cleared");
  }

  if (cmd_1 == "color") {
      String color_str = str_split(cmd, 2);
      int color_r = str_int(color_str, 0, ',');
      int color_g = str_int(color_str, 1, ',');
      int color_b = str_int(color_str, 2, ',');
      int brightness = str_int(color_str, 3, ',');
      if (brightness > -1) {
        #ifdef ESP32_LED_H
          set_brightness(brightness);
        #endif
      }

      #ifdef ESP32_LED_H
        setAllColor(color_r, color_g, color_b);
      #endif

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
        #ifdef ESP32_LED_H
          set_brightness(brightness);
        #endif
      }
      #ifdef ESP32_LED_H
        set_pixel(x, y, color_r, color_g, color_b);
      #endif

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
        #ifdef ESP32_LED_H
          set_brightness(brightness);
        #endif
      }
      #ifdef ESP32_LED_H
        set_pixel_i(i, color_r, color_g, color_b);
      #endif

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
        #ifdef ESP32_LED_H
          set_brightness(brightness);
        #endif

        return send_body(jsonField("brightness", String(brightness), false));
      }
  }

  
  return send_msg("received");
}

bool HandleResetCommand(String& cmd) {
  String cmd_1 = str_split(cmd, 1);
  if (cmd_1 == "counters") {
    reset_counters();
    return send_msg("counters cleared");
  }
  
  resetFunc();

  return send_msg("cleared");
}

#pragma endregion Handlers

// Runs on Core 0
bool OnApiCommand(String& cmd) {
  String first_word = str_split(cmd, 0);
  
  if (first_word == "set") {
    return HandleSetParameter(cmd);
  }

  if (first_word == "lcd") {
    return HandleLcdCommand(cmd);
  }

  if (first_word == "led") {
    return HandleLedCommand(cmd);
  }

  if (first_word == "reset") {
    return HandleResetCommand(cmd);
  }

  if (first_word == "p") {
    String pos_str = str_split(cmd, 1);
    int x = str_int(pos_str, 0, ',');
    int y = str_int(pos_str, 1, ',');

    String clr_str = str_split(cmd,2);
    int r = str_int(clr_str, 0, ',');
    int g = str_int(clr_str, 1, ',');
    int b = str_int(clr_str, 2, ',');
    int a = str_int(clr_str, 3, ',');
    #ifdef ESP32_LED_H
      set_pixel(x, y, r, g, b);
      return send_msg("pixel set");
    #endif
    return send_msg("ESP32_LED_H not included");
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
    return send_msg("set timer [" + String(timer_id) + "] to " + String(timer_delay) + " ms");
  }

  if (first_word == "mem") {
    uint32_t mem = ESP.getFreeHeap();
    Serial.print(F("FreeHeap:"));
    Serial.println(mem);
    return send_body(jsonField("mem", String(mem), false));
  }

  if (first_word == "sitrep") {
    String res = "";

    res += jsonField("mem", String(ESP.getFreeHeap()), true);
    res += jsonField("mode", String(mode), true);
    res += jsonField("show_rssi", String(show_rssi), true);
    res += jsonField("lcd_show_network", String(lcd_show_network), true);
    res += jsonField("lcd_show_rssi", String(lcd_show_rssi), true);
    res += jsonField("serial_debug", String(serial_debug), true);
    res += jsonField("rssi", String(WiFi.RSSI()), true);
    res += jsonField("ipaddress", String(IPADDRESS), true);
    res += jsonField("macaddress", String(MACADDRESS), true);
    res += jsonField("counter_0", String(counters[0]), true);
    res += jsonField("counter_1", String(counters[1]), true);
    res += jsonField("counter_2", String(counters[2]), true);
    res += jsonField("counter_3", String(counters[3]), true);
    res += jsonField("counter_4", String(counters[4]), true);
    res += jsonField("counter_5", String(counters[5]), true);
    res += jsonField("counter_6", String(counters[6]), true);
    res += jsonField("counter_7", String(counters[7]), false);
    
    return send_body(res);
  }

  if (first_word == "scan_i2c") {
    byte *addresses = scan_i2c();
    uint8_t length = sizeof(addresses);

    Serial.println(String(sizeof(addresses[0])));
    Serial.println(String(sizeof(addresses)));
    
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
    url += "?src=" + urlEncode("127.0.0.1");
    //String response = net_post(url, buffer);

    //String response;
    //post_data(MACADDRESS, "type0", "arg0", "val_0x01", response);

  }

  return false;
}


