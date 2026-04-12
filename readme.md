# ESP32 Web Controller

A C/Arduino firmware for the ESP32 that connects to your WiFi network and exposes a **REST API + web interface** for reading and writing hardware I/O in real time — control GPIO pins, sensors, LED matrices, servos, tone outputs, and more from any browser or HTTP client on your network.

---

## Features

- **WiFi REST API** — full HTTP endpoint set for analog/digital read & write, servo control, tone generation, and more
- **Built-in web interface** — browser-based control panel served directly from the ESP32 (no external server needed)
- **LED matrix support** — set individual pixels by (x, y) coordinate or index, with full RGB and brightness control
- **Sensor support** — DHT temperature/humidity, DS18B20 OneWire temperature, motion/microwave sensors, ultrasonic range
- **LCD display** — 20x4 character LCD with backlight control, row/column printing, RSSI and network status display
- **Dual-core architecture** — WiFi/HTTP server runs on Core 0, sensor polling and hardware I/O run on Core 1 (FreeRTOS)
- **Timer system** — configurable non-blocking timers for polling intervals and periodic tasks
- **Modular design** — hardware modules (LED, LCD, DHT, OneWire) are opt-in via `#include` — only compile what you need
- **I2C scanner** — built-in utility to discover I2C device addresses on the bus
- **Sitrep endpoint** — single call returns full device status: memory, mode, RSSI, IP, MAC, counters

---

## Hardware Support

| Module | Hardware |
|---|---|
| Temperature & Humidity | DHT11 / DHT22 |
| Temperature (precision) | DS18B20 (OneWire) |
| Motion | Microwave / PIR sensors |
| Display | 20x4 I2C LCD |
| LED Matrix | WS2812B / NeoPixel (via FastLED) |
| Distance | HC-SR04 Ultrasonic |
| Servos | Standard PWM servo motors |
| Audio | Buzzer / tone output |

---

## API Endpoints

### GET

| Endpoint | Description |
|---|---|
| `GET /id` | Returns device MAC address |
| `GET /` | Serves LED matrix web interface |
| `GET /ctrl` | Serves main control web interface |

### POST

| Endpoint | Params | Description |
|---|---|---|
| `POST /digitalout` | `pin`, `value` | Digital write to a pin (0 or 1) |
| `POST /analogout` | `pin`, `value` | Analog write (PWM) to a pin |
| `POST /analogin` | `pins` | Read analog values from one or more pins |
| `POST /servo` | `pin`, `value` | Write servo position |
| `POST /tone` | `pin`, `value` | Play tone at frequency (0 = stop) |
| `POST /pulse` | `pin`, `value`, `time` | Output a timed pulse in microseconds |
| `POST /sweep` | `servo`, `pwm`, `value`, `low`, `high`, `count`, `delay` | Sweep servo or PWM through a range |
| `POST /mat` | `x`, `y`, `r`, `g`, `b`, `h` | Set LED matrix pixel by (x,y) with RGB color |
| `POST /integer` | `pin`, `value` | Write integer value to pin |
| `POST /data` | `src`, `type`, `var`, `val` | Receive sensor data from another device |
| `POST /api` | `cmd` | Text command interface (see below) |

### API Command Interface (`POST /api`)

Send commands as plain text via the `cmd` parameter:

```
digital <pin> <value>      # Digital write  (alias: di)
analog <pin> <value>       # Analog write   (alias: an)
tone <pin> <value>         # Tone output    (alias: to)
lcd init                   # Initialize LCD
lcd clear                  # Clear LCD
lcd bl on|off              # LCD backlight
lcd print <text> <row> <col>
lcd show ip|mac|version|rssi
led clear                  # Clear LED matrix
led color <r,g,b,brightness>
led set <x,y> <r,g,b>     # Set pixel by coordinate
led seti <i> <r,g,b>      # Set pixel by index
led brightness <value>
mode <n>                   # Set operating mode
timer <id> <ms>            # Set timer interval
mem                        # Report free heap memory
sitrep                     # Full device status report
reset                      # Reboot device
reset counters             # Reset API counters
```

---

## Getting Started

### Requirements

- ESP32 development board
- [Arduino IDE](https://www.arduino.cc/en/software) with ESP32 board support installed
- Libraries (install via Arduino Library Manager):
  - `ArduinoJson`
  - `UrlEncode`
  - `FastLED` (if using LED matrix)
  - `DHT sensor library` (if using DHT sensor)
  - `OneWire` + `DallasTemperature` (if using DS18B20)
  - `LiquidCrystal_I2C` (if using LCD)

### Setup

1. Clone or download this repository
2. Open `esp32_webcontroller.ino` in Arduino IDE
3. Copy `secrets.h.example` to `secrets.h` and fill in your WiFi credentials:
   ```c
   const char* ssid     = "YOUR_SSID";
   const char* password = "YOUR_PASSWORD";
   ```
4. In `esp32_webcontroller.h`, uncomment the modules you want to use:
   ```c
   //#include "esp32_led.h"
   //#include "esp32_lcd.h"
   //#include "esp32_dht.h"
   //#include "esp32_onewire.h"
   ```
5. Flash to your ESP32
6. Open Serial Monitor (115200 baud) — the device IP address will be printed on connection
7. Navigate to `http://<device-ip>` in your browser

---

## Architecture

The firmware uses the ESP32's dual-core capability via FreeRTOS:

- **Core 0** — WiFi connection, HTTP server, handles all incoming API requests
- **Core 1** — Main Arduino loop, sensor polling, hardware I/O, LCD updates, timer management

This separation ensures the web server remains responsive even during hardware polling operations.

Hardware modules are conditionally compiled using `#ifdef` guards — if a module header is not included, its code is completely excluded from the build, keeping firmware size minimal.

---

## Project Structure

```
esp32_webcontroller/
├── esp32_webcontroller.ino   # Main sketch — setup, loop, API command handlers
├── esp32_webcontroller.h     # Core controller — server routes, WiFi setup, dual-core task
├── esp32_server.h            # HTTP server helpers, JSON response builders
├── esp32_net.h               # Outbound HTTP POST (for reporting data to external server)
├── esp32_helpers.h           # String utilities, JSON builder, I2C scanner, indicator LED
├── esp32_timer.h / .cpp      # Non-blocking timer system
├── esp32_led.h               # WS2812B / NeoPixel LED matrix support
├── esp32_lcd.h               # I2C LCD display support
├── esp32_dht.h               # DHT temperature/humidity sensor
├── esp32_onewire.h           # DS18B20 OneWire temperature sensor
├── esp32_site.h              # Embedded web UI (HTML/CSS/JS served from flash)
├── index.html / .js / .css   # Web interface source files
├── matrix.html               # LED matrix control interface
├── secrets.h                 # WiFi credentials (not committed)
└── package.json              # Minification build script
```

---

## License

MIT License — free to use, modify, and distribute.
