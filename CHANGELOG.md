# Changelog

All notable changes to this project will be documented in this file.


## [Unreleased]

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



[unreleased]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.2.0...HEAD
[1.2.0]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.1.2...v1.2.0
[1.1.2]: https://github.com/simonttp78/FlyballETS-Software/compare/v1.0.0...v1.1.2
[1.0.0]: https://github.com/simonttp78/FlyballETS-Software/releases/tag/v1.0.0
