## FlyballETS-Software by simonttp78

This project is a fork of Flyball ETS project by vyruz1986.
ETS stands for Electronic Training System and it's kind of Electronig Judging System (EJS - used during official Flyball competitions) adjusted to regular Flyball trainings needs.

## Change log

#### Check [CHANGELOG.md](https://github.com/simonttp78/FlyballETS-Software/blob/master/CHANGELOG.md) file

## Usage instruction
- While powering on the system Frimware version is shown on LCD;
- Short press of Laser button activates two red LEDs for ETS setup/positioning. Default time is 180s, but it can be changed via WebUI Config menu;
- Long press (detected after button release) of Laser button turns WiFi OFF or ON;
- Short press of Mode button switches accuracy between 2 or 3 digits;
- Long press of Mode button changes run direction (which sensors set is for start/finish line). Direction is indicated on LCD in form of ">" or "<" signs;
- Remote control button 1 is used to START or STOP the race (if auto-stop function won't applly for whatever reason);
- Remote control button 2 is used to RESET the race. Race has to be stopped first;
- Remote control buttons 3-6 are used for manual triggering of dogs' faults (if auto-fault detection funtion won't apply, e.g. dog runs outside the gate or drops the ball). This function works only when race is in STARTING or RUNNING state;
- When race is in RESET (READY on LCD) state remote control 3-6 buttons are used for defining number of running dogs if it's less than 4;
- When race is in RESET (READY on LCD) state long press of button 3 will toggle reruns function OFF/ON. When reruns are truned off ETS is not expecting dogs to correct/rerun their faults (each dog will run only once and race will be stopped). Reruns set to OFF are highlighted on LCD by "*X" at the end of each dog time line;
- When race is in RESET (READY on LCD) state long press of button 4 will toggle between NAFA or FCI starting sequence;
- It's possible to use SD card for storing races results. SD card needs to be formatted in FAT32. Plugin in or out SD card while ETS system is on, will trigger auto-reboot. Properly detected SD card is indicated on LCD with "sd" letters;
- SD card has tag file storing unique ID for each power-on session with at least one start/stop sequence (race). Every race in the sesssion will have own race id that is displayed on LCD (top right corner) and will be part of CSV file with results that is written to SD card. In addition to races resuls, each race sensors readings will be stored on SD card too in folder SENSORS_DATA. This is usefull for offline debugging in case malfunction would happen;
- It's possible to change decimal separator for CSV file (SD card) to comma ',' (used in most of non english speaking countries) via WebUI Configuration menu;
- When GPS communication is established there will be "G" letter visible on LCD. GPS time is used mainly for timestamp while saving races data to SD card. When GPS is not active dummy timestamp of 2021-01-01 13:00:00 will be used (CET time is default time. Other popular timezones will be added in future Frimware versions);
- WebUI is accessible after connecting to WiFi transmitted by ETS by entering IP address: 192.168.20.1 in web browser


## Source files

The source files are divided into 3 subprojects:

### Firmware

Main folder contains the sources for compiling the firmware which is to be flashed onto the ESP32 MCU.
It uses [PlatformIO](https://platformio.org/) to manage the board library, as well as other libraries needed.

### WebUI

This folder contains the angular sources for building the web interface used to control the system.
They need to be built first because they output a header file to the `include` folder which is needed in order to compile the firmware.
Make sure you have [NodeJS](https://nodejs.org/en/download/) installed, then run the following commands in the `WebUI` folder:

```bash
npm install
npm run build
```

IOS USERS!!!
In iOS 15 Safari setting => Advanced => Experimental Features => NSURLSession WebSocket is default On. It has to be turned Off, otherwise WebUI will be crashing.

Since HW 5.x.x. rev.S that is compatibile with project version 0.9.x and 1.x.x there is option to keep WebUI source files stored on SD card instead of ESP32 LoLin32 flash.

### Test

In folder "test" there is python test script and input/scenario files that are used for functional and regression testing of sensors reading interpreting algorithm. Testing mode require firmware compilation with with Simulate flag set to true.

## Thanks

This project was made possible by the following awesome libraries/projects and their developers:

- [TinyGPS++](http://arduiniana.org/libraries/tinygpsplus/) for interacting with the GPS module
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) for easily building JSON objects in Arduino
- [ESPAsyncWebServer](https://github.com/BlueAndi/ESPAsyncWebServer) fork by BlueAndi of very powerful and featured webserver & websocket interface with improvements
- [Arduino core SDK for ESP32](https://github.com/espressif/arduino-esp32) for being able to use the ESP32 platform at all
- [Embedis](https://github.com/thingSoC/embedis) for providing an easy to use EEPROM interface to store settings
- [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) for reliably controlling WS2812 LEDs using ESP32
- [Timezone](https://github.com/JChristensen/Timezone) used for local time adjustments: time zones and daylight saving time change
- [AsyncElegantOTA](https://github.com/ayushsharma82/AsyncElegantOTA) used for firmware update via WebUI
