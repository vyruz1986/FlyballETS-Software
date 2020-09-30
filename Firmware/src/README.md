## FlyballETS-Software

The FlyballETS-Software project is a subproject of the [Flyball ETS project](https://sparkydevices.wordpress.com/tag/flyball-ets/).
It contains the sources for the Arduino Pro Mini microcontroller which can be considere the heart of the system.

## Source files

The source files consist of a 'central' FlyballETS-Software.ino file, and several _.cpp/_.h files.
The ino file is left in place for Arduino IDE compatibility, the other files contain classes which are used throughout the project.

You will see there are also 2 Visual Studio files, FlyballETS-Software.sln and .vcxproj, these are designed to be used for people who (like me) use Visual Studio to code.

## Requirements

The source files are 100% Arduino compatible, this means you can compile them by just installing the latest Arduino IDE and opening the FlyballETS-Software.ino file.
You do need the ESP32-Arduino SDK however, see [this link](https://github.com/espressif/arduino-esp32) for details and instructions.

## Thanks

This project was made possible by the following awesome libraries/projects and their developers:

- [TinyGPS++](http://arduiniana.org/libraries/tinygpsplus/) for interacting with the GPS module
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) for easily building JSON objects in Arduino
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) for providing a very powerful and featured webserver & websocket interface
- [Arduino core SDK for ESP32](https://github.com/espressif/arduino-esp32) for being able to use the ESP32 platform at all
- [Embedis](https://github.com/thingSoC/embedis) for providing an easy to use EEPROM interface to store settings
- [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) for reliably controlling WS2812 LEDs using ESP32
- [Espurna Project](https://bitbucket.org/xoseperez/espurna) was used as a great example to solve several problems, such as the web UI and the settings interface.
