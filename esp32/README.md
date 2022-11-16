# esp32

The code running on the ESP32, built and uploaded with [PlatformIO](https://platformio.org/).

## Pinout

The default suggestions from [GxEPD2](https://github.com/ZinggJM/GxEPD2):

|Waveshare | LOLIN D32 Pro | 
|----------|---------------|
|Vcc       | 3V            |
|GND       | GND           | 
|DIN       | 23 (MOSI)     | 
|CLK       | 18 (SCK)      |
|CS        | 5             | 
|DC        | 0             | 
|RST       | 2             | 
|BUSY      | 15            | 

## Build

From repository root folder: `task build-esp`

## Build and upload

From repository root folder: `task upload-esp`

## Monitor

From repository root folder: `task monitor-esp`

## Configuration

Configuration is done in `config.cpp`.
