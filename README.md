## FlyballETS-Software

The FlyballETS-Software project is a subproject of the [Flyball ETS project](https://sparkydevices.wordpress.com/tag/flyball-ets/).
It contains the sources for the Arduino Pro Mini microcontroller which can be considere the heart of the system.

## Source files

The source files consist of a 'central' FlyballETS-Software.ino file, and several *.cpp/*.h files.
The ino file is left in place for Arduino IDE compatibility, the other files contain classes which are used throughout the project.

You will see there are also 2 Visual Studio files, FlyballETS-Software.sln and .vcxproj, these are designed to be used for people who (like me) use Visual Studio to code.

## Requirements

The source files are 100% Arduino compatible, this means you can compile them by just installing the latest Arduino IDE and opening the FlyballETS-Software.ino file.
**One small change is required!** Since I have used some features which are only available since C++11 (Arduino IDE uses C++98 by default).
To modify your Arduino IDE installation to support these features, follow [this answer on Stackoverflow.](http://stackoverflow.com/a/26175586/1177847), it's as simple as modifying one single text file.
