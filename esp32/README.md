# esp32

The code running on the ESP32, built and uploaded with [PlatformIO](https://platformio.org/).

## Pinout

The default suggestions from [GxEPD2](https://github.com/ZinggJM/GxEPD2):

| Waveshare | LOLIN D32 Pro |
| --------- | ------------- |
| Vcc       | 3V            |
| GND       | GND           |
| DIN       | 23 (MOSI)     |
| CLK       | 18 (SCK)      |
| CS        | 5             |
| DC        | 0             |
| RST       | 2             |
| BUSY      | 15            |

## Build

From repository root folder: `task build-esp`

Secrets are read from the environment

- `WIFI_SSID` - WiFi network name
- `WIFI_PASSWORD` - WiFi password
- `BACKEND_HOST` - backend base URL, e.g. `http://192.168.1.1:8080`
- `BACKEND_TOKEN` - optional value for the `Authorization` header
- `DISPLAY_LOCATION` - place name in the footer, at most 15 characters
- `DISPLAY_PADDING_TOP`, `_RIGHT`, `_BOTTOM`, `_LEFT` - optional inset in
  pixels from each panel edge. Dfaults to 20.

## Build and upload

From repository root folder: `task upload-esp`

## Fitting a frame

From repository root folder: `calibrate-esp`

This flashes firmware that draws a ruler at the middle of each edge instead of
the dashboard. Put the panel in the frame, note the first visible tick on each
edge, and set `DISPLAY_PADDING_*` to that plus the margin you want. Then run
`upload-esp` to restore the dashboard.

## Monitor

From repository root folder: `task monitor-esp`

## Configuration

Configuration is done in `src/config.h`.
