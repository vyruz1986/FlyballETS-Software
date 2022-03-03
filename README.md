## FlyballETS-Software by simonttp78

This project is a fork of Flyball ETS project by vyruz1986.
ETS stands for Electronic Training System and it's kind of Electronig Judging System (EJS - used during official Flyball competitions) adjusted to needs of regular Flyball trainings.

## Change log

#### Check [CHANGELOG.md](https://github.com/simonttp78/FlyballETS-Software/blob/master/CHANGELOG.md) file

## Source files

The source files are divided into 2 subdirectories:

### Firmware

This folder contains the sources for compiling the firmware which is to be flashed onto the ESP32 MCU.
It uses [PlatformIO](https://platformio.org/) to manage the board library, as well as other libraries needed.

### WebUI

This folder contains the angular sources for building the web interface used to control the system.
They need to be built first because they output a header file to the `Firmware` folder which is needed in order to compile the firmware.
Make sure you have [NodeJS](https://nodejs.org/en/download/) installed, then run the following commands in the `WebUI` folder:

```bash
npm install
npm run build
```

IOS USERS!!!
In iOS 15 Safari setting => Advanced => Experimental Features => NSURLSession WebSocket is default On. It has to be turned Off, otherwise WebUI will be crashing.

Since HW 5.x.x. rev.S that is compatibile with project version 0.9.x and 1.x.x there is option to keep WebUI source files stored on SD card instead of ESP32 LoLin32 flash.

## Thanks

This project was made possible by the following awesome libraries/projects and their developers:

- [TinyGPS++](http://arduiniana.org/libraries/tinygpsplus/) for interacting with the GPS module
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) for easily building JSON objects in Arduino
- [ESPAsyncWebServer](https://github.com/BlueAndi/ESPAsyncWebServer) fork by BlueAndi of very powerful and featured webserver & websocket interface with improvements
- [Arduino core SDK for ESP32](https://github.com/espressif/arduino-esp32) for being able to use the ESP32 platform at all
- [Embedis](https://github.com/thingSoC/embedis) for providing an easy to use EEPROM interface to store settings
- [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) for reliably controlling WS2812 LEDs using ESP32
- [Espurna Project](https://github.com/xoseperez/espurna) was used as a great example to solve several problems, such as the web UI and the settings interface.
- [Timezone](https://github.com/JChristensen/Timezone) used for local time adjustments: time zones and daylight saving time change
- [AsyncElegantOTA](https://github.com/ayushsharma82/AsyncElegantOTA) used for firmware update via WebUI
