#include "esp32_webcontroller.h"
#include <ArduinoJson.h>

uint8_t mode = 1;
bool show_rssi = false;

bool lcd_show_network = true;
bool lcd_show_rssi = true;
bool serial_debug = false;

int Wifi_Signal;
String _net_response;

#pragma region Setup

void SetupTimers() {
  timers.AddTimer(0, 1000);
  timers.AddTimer(1, 3000);
  timers.AddTimer(2, 5000);
  timers.AddTimer(3, 50);
}

void PreSetup() {
  #ifdef ESP32_LCD_H
    lcd_init();
  #endif
  
  #ifdef ESP32_LED_H
    led_init();
  #endif
  
  #ifdef ESP32_DHT_H
    dht_init();
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
	pinMode(13, INPUT);
  pinMode(12, OUTPUT);

}
  
#pragma endregion Setup

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

int microwave_0;
int microwave_1;
float* temps;
float temp, hum;

#pragma endregion Testing

uint32_t motion_0 = 0;
uint32_t motion_1 = 0;

int httpStatus;
String httpResponse;
StaticJsonDocument<128> doc;
uint16_t motion;
uint32_t mem;
bool alert_sent = false;

// Runs on Core 1
void loop(void) {

  #ifdef ESP32_LED_H
    if (led_loop_text != "") { 
      loop_text(led_loop_text);
    }
  #endif

  // motion = digitalRead(13);
  // if (motion == 1) {
  //   if (alert_sent == false) {
  //     net_post("http://192.168.0.36/api?cmd=motion", "", httpResponse);
  //     alert_sent = true;
  //     digitalWrite(12, HIGH);
  //     lcd_write(0, 0);
  //   }
  // } else {
  //   if (alert_sent == true) {
  //     alert_sent = false;
  //     digitalWrite(12, LOW);
  //     lcd_print(" ", 0);
  //   }

  // }

  if (doBlink) {
    Blink();
    doBlink = false;
  }

  #ifdef ESP32_LCD_H
    if (timers.CheckTimer(0)) {
      lcd_print(String(counters[0]) + "|" + String(counters[1]) + "|" + String(counters[2]), 3);

      Wifi_Signal = WiFi.RSSI();
      mem = ESP.getFreeHeap();

      lcd_print_r(str_pad_s(String(mem) + " M", 9), 2);
      lcd_print_r(String(Wifi_Signal) + " dBm", 3);

      #ifdef ESP32_DHT_H
        dht_getvalues(temp, hum);
        lcd_print_r(str_pad_s((String(temp) + " F"), 8), 0);
        lcd_print_r(str_pad_s((String(hum) + " %"), 8), 1);
      #endif

    }
  #endif


  #ifdef ESP32_LCD_H
    if (timers.CheckTimer(1)) {
      httpStatus = net_get("http://192.168.0.30/data?src=dht", httpResponse);
      if (httpStatus == HTTP_CODE_OK){
        DeserializationError error = deserializeJson(doc, httpResponse);
        if (!error) {
          lcd_print(str_pad_s(String(doc["temp"]) + " F", 8), 0);
          lcd_print(str_pad_s(String(doc["hum"]) + " %", 8), 1);
        } else {
          counters[1]++;
        }
      } else {
        lcd_print(str_pad("NetErr:" + net_status_code(httpStatus), 8), 0);
        lcd_print(str_pad("NetErr:" + net_status_code(httpStatus), 8), 1);
        counters[1]++;
      }
      

      httpStatus = net_get("http://192.168.0.35/data?src=onewire", httpResponse);
      if (httpStatus == HTTP_CODE_OK){
        DeserializationError error = deserializeJson(doc, httpResponse);
        if (!error) {
          lcd_print_r(str_pad_s(String(doc["temp"]) + " F", 12), 0);
        } else {
          counters[1]++;
        }
      } else {
        lcd_print_r(str_pad("NetErr:" + net_status_code(httpStatus), 8), 0);
        counters[1]++;
      }

      httpStatus = net_get("http://192.168.0.33/data?src=onewire", httpResponse);
      if (httpStatus == HTTP_CODE_OK){
        DeserializationError error = deserializeJson(doc, httpResponse);
        if (!error) {
          lcd_print_r(str_pad_s(String(doc["temp"]) + " F", 12), 1);
        } else {
          counters[1]++;
        }
      } else {
        lcd_print_r(str_pad("NetErr:" + net_status_code(httpStatus), 8), 1);
        counters[1]++;
      }

    }

    // if (timers.CheckTimer(2)) {
    //   httpStatus = net_get("http://192.168.0.35/data?src=onewire", httpResponse);
    //   if (httpStatus == HTTP_CODE_OK){
    //     DeserializationError error = deserializeJson(doc, httpResponse);
    //     if (!error) {
    //       lcd_print_r(str_pad_s(String(doc["temp"]) + " F", 12), 0);
    //     } else {
    //       counters[1]++;
    //     }
    //   } else {
    //     lcd_print_r(str_pad("NetErr:" + net_status_code(httpStatus), 8), 0);
    //     counters[1]++;
    //   }
    // }

    // if (timers.CheckTimer(3)) {
    //   httpStatus = net_get("http://192.168.0.33/data?src=onewire", httpResponse);
    //   if (httpStatus == HTTP_CODE_OK){
    //     DeserializationError error = deserializeJson(doc, httpResponse);
    //     if (!error) {
    //       lcd_print_r(str_pad_s(String(doc["temp"]) + " F", 12), 1);
    //     } else {
    //       counters[1]++;
    //     }
    //   } else {
    //     lcd_print_r(str_pad("NetErr:" + net_status_code(httpStatus), 8), 1);
    //     counters[1]++;
    //   }
    // }

  #endif

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
          //set_brightness(brightness);
        #endif
      }

      #ifdef ESP32_LED_H
        //setAllColor(color_r, color_g, color_b);
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
          //set_brightness(brightness);
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

  if (first_word == "motion") {
    #ifdef ESP32_LCD_H
      lcd_print("motion", 2);
    #endif
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

  return false;
}
