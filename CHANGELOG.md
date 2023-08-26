# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added

### Changed


## [1.13.0] - 2023-08-26

### Added
- Detection of wrongly configured Run Direction (TC 50)
- Added detection of starting dog that missed gate while entering (TC 49)

### Changed
- Essential stability fix (ESPAsyncWebServer library)
- LCD and WebUI refresh optimization
- Fix for wrong presentation of FW version on LCD
- Fix for huge early cross scenario
- Improved interrupts processing (spinlock)
- Improved simulation function
- Fix for 4 digits RaceID (LCD problem)
- SDK update

## [1.11.10] - 2023-03-22

### Added
- new Test Cases: 47 and 48
- fix for TC 47 (S2 noise after dog coming back)
- fix for TC 48 (S1 noise after dog coming back after early cross)

### Changed
- fix for fault light on (purple) in WebUI after reset


## [1.11.9] - 2023-03-05

### Added
- timeout support in testETS (1.0.1)
- uptime command for testETS (1.1.0)
- support for WebUI "up time" higher than 10h (stability tests)
- removed old TC 43 and added new TC 46

### Changed
- fix for false cross fault detection in Ultra 15-22 race (TC 46)
- testETS scripts corrections and small refactoring (1.0.0)
- pre-filtering waiting time changed from 15ms to 12ms
- optimization of main program loop
- optimization of LCD refresh
- fixed WebUI time in case of no GPS connection
- removed unused historical races data (fix for stability tests)
- max number of races changed from 100 to 1000 (fix for stability tests)
- testETS summary closed after each testcase (1.0.2)
- testETS "stab" migration to "loop"

## [1.10.2] - 2022-10-27

### Added
- runtime update of accuracy and run direction from WebUI config menu (no need to reset)
- fake dogs times (next dog did unmeasurable fault) marked with # in results

### Changed
- Net time renamed to Clean Time (CT) and has value only for clean races otherwise "n/a" is shown
- corrected option "all" bug in testESP.py

## [1.9.0] - 2022-10-15

### Added
- INSTRUKCJA_PL.md updated with recommendation in relation to sunlight and Wi-Fi settings
- First issue of python test script for ETS (in simulation mode)
- Run direction and firmware version visible in WebUI (system data section)
- Python tests (thanks to Controll17)

### Changed
- WebUI main Race and Configuration pages updated (with support from MiKoKappa)
- Fix for Light0 (fault light) is not truning off in WebUI
- INSTRUKCJA_PL.md updated with details about symptoms collection in case of faults
- Filtering time of early dog while invisible dog is running changed from 4.5s to 5.5s (fix for 89-13)
- Repositorium structure change in order to simplify development work and resolve library dependencies problems
- Logs format change

## [1.6.0] - 2022-06-23

### Added
- OTA Firmware update progress on LCD
- Option to use comma ',' as decimal separator in CSV file (SD card) via WebUI Congifuration menu

### Changed
- corrected dogs re-runs off state on LCD after number of running dogs change
- code refactoring in main.h/cpp, RaceTime and NetTime
- fix for proper handling of csv with tag 1
- FCI / NAFA staring sequence separated from 2 / 3 digits accuracy
- Short press of mode button now changes accuracy, without chaging starting sequence
- FCI / NAFA starting sequence can be changed by long press of button 4 on remote control or via WebUI Configuration menu

## [1.5.3] - 2022-06-13

### Added
- Polish user manual updated with chapter "Światła" and information about remote battery
- LCD screen indicator for active WiFi (letter "W")
- fix for invisible dog with perfect crossing (TC 31)
- fix for fake dog 1 time due to sensors noise (TC 37 and TC 41)
- fix for big cross treated as sceanrio TC 40 (TC 46)

### Changed
- improved detection of false "ok" crossing to cover true cross up to 2s
- minor code refactoring (GPS and pins)
- LCD screen indicator for active GPS connection changed from "gps" to "G"
- improved noise filtering function
- fix for false "ok" crossing of last dog in case of sensors noise
- simulated testcases
- debug log level changed to "info"


## [1.4.1] - 2022-05-21

### Added

- 2 new simulated races
- polish instruction of use
- added wifi on/off via console command

### Changed

- Changed GPS module pinout (TX pin 36 swapped with PPS pin 22). HW patch needed, but not critical
- Refactoring ("if" and "boolean")
- ISR function update to avoid watchdog timeouts (ETS could reset while in "READY" state and sensors triggered)
- fix for PowerOnTag always 0 in WebUI
- fix for reverted logic of Re-runs OFF columnt in csv (SD card)
- small logs cleanup due to SDK update
- 2 racing algorithm fixes (fault after invisible dog)
- SD card column "reruns off" update
- frequency of GPS time update changed from 5s to 10s
- OTA update password now same as Admin password
- SDK and libraries update

## [1.2.1] - 2022-04-14

### Added

- Long press of laser button toggling Wifi state OFF and ON
- Possibility to deactivate re-runs (race will stop without expecting dogs to correct/reruns their faults)
- Simulated race 44 and deactivation of filtering time 350ms for sensors reading in case of first entering dog
  This is not real case sceanrio, but without it human simulating dog enter could cause confusing OK enter time
- Number of racign dogs and re-runs off indicator added to race data saved on SD card

### Changed

- Sensors prints to console and SD card now include also empty entries for easier simulation usage
- Fixed problem with SD card sensors file tag and first race prints
- WiFi related console prints cleanup

## [1.2.0] - 2022-03-10

### Added

- Possibility to deactivate re-runs (race will stop without expecting dogs to correct/reruns their faults)

### Changed

- Buttons handling function with possibility to detect short and long button press


## [1.1.2] - 2022-03-03

### Added

- Added possibility to define number of racing dogs (remote control + WebUI)
- Laser ON time now configurable via WebUI config
- Two additional sensor noise filtering functions

### Changed

- Side Switch button renamed to Mode button. Single press --> Mode change (NAFA, FCI). Double press --> Run direction switch
- Websocket interface optimisation by reducing size of RaceData JSON
- main.cpp refactoring
- Sensor filtering time changed from 5ms to 6ms
- Migratino to Angular 13

## [1.0.0] - 2022-02-16

### Added

- Two modes configurable in settings:
  - FCI lights with 2 digits accuracy
  - NAFA lights with 3 digits accuracy
- "perfect crossing" (below 5ms)
- Simulated scenarios for code functional and regression testing
- New console commands for runtime interfacing and test automation
- PISO register for interfacing with remote control and buttons: released ESP32 lines used for SD card communication
- SD card support
  - saving race data to file
  - saving sensors reading to file
- GPS PPS line support - readiness for system time synchronization
- Battery calibration improvements
- Local system time
- Reset and Manual stop indicated by white fault light blink

### Changed

- Total crossing time replaced with Net time
- Modified main race handling algorithm to be in line with FCI EJS requirements:  only S1 line used for time calculations
- Re-run algorithm changes to assure proper order of re-running dogs
- Code optimizations to reduce non essential tasks execution while race is running
- Modified lights set-up and starting sequence in order to assure more accurate sync between lights and race start
- WebUI and LCD layout changes

### Removed

- Removed support for legacy lights: only WS2811B type lights supported



[unreleased]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.13.0...HEAD
[1.13.0]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.11.10...v1.13.0
[1.11.10]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.11.9...v1.11.10
[1.11.9]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.10.2...v1.11.9
[1.10.2]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.9.0...v1.10.2
[1.9.0]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.6.0...v1.9.0
[1.6.0]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.5.3...v1.6.0
[1.5.3]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.4.1...v1.5.3
[1.4.1]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.2.1...v1.4.1
[1.2.1]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.1.2...v1.2.0
[1.1.2]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.0.0...v1.1.2
[1.0.0]: https://github.com/simonttp78/FlyballETS-Software/releases/tag/v1.0.0
