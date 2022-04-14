## FlyballETS-Software by simonttp78

This project is a fork of Flyball ETS project by vyruz1986.
ETS stands for Electronic Training System and it's kind of Electronig Judging System (EJS - used during official Flyball competitions) adjusted to needs of regular Flyball trainings.

## Change log

#### Check [CHANGELOG.md](https://github.com/simonttp78/FlyballETS-Software/blob/master/CHANGELOG.md) file

## Basic use instruction
- During powering on Frimware version is visible on LCD
- Short press of Laser button activate two red LEDs for ETS setup/positioning. Default time is 60s, but it can be changed via WebUI Config menu
- Long press of Laser button turns WiFi OFF or ON
- Short press of Mode button will switch betweeen modes: FCI with 2 digits accuracy and NAFA with 3 digits accuracy. New mode name is visible for short time on LCD.
- Long press of Mode button change run direction (which sensors are use for mesuring and which for cross detection). Direction is indicated on LCD in form of ">" or "<" signs
- Remote control button 1 is used to START or STOP the race (if auto-stop function won't applly for some reason)
- Remote control button 2 is used to RESET the race. Race has to be first stopped.
- Remote control buttons 3-6 are used to trigger manually dog faults (if auto-fault detection funtion won't apply, e.g. dog run outside the gate or drop the ball). This function work only when race is starting or running.
- When race is in RESET (READY) state remote control 3-6 buttons are used to define number of running dogs in race as during trainings less then 4 dogs might be racing
- When race is in RESET (READY) state long press of button 3 will toggle reruns function OFF/ON. When reruns are truned off ETS is not expecting dogs to correct/rerun their faulty runs (each dog will run only once and race will be stopped). Reruns set to OFF are highlighted on LCD by "*X" at the end of each dog time line.
- SD card need to be formatted in FAT32. Plging in or out SD card while ETS system is on will trigger auto-reboot. Properly detected SD card is indicated on LCD with "sd" letters.
- On SD card there is tag file used to generate unique ID for each power-on session. Every race in the sesssion got race id that is stored in CSV file with results on SD card and displayed in top right corner of LCD. Beside race results in csv file in folder SENSORS_DATA also sensors readings are stored for debugging purposes.
- When GPS communication is established there will be "gps" letters visible on LCD. GPS time is used mainly for timestamp in races data saving to SD card. When GPS is not active logging will alway start with dummy timestamp of 2021-01-01 13:00:00 (CET time is default time. Other popular timezones will be added in future Frimware versions)


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
