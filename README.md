# eink-morning-announcements

Morning announcements[^1] on a eink display! Displays upcoming calendar events, weather forecasts and custom messages.

![The display](https://github.com/engvik/eink-morning-announcements/raw/main/assets/display.png "The display")

This is built with a [Lolin D32 Pro](https://www.wemos.cc/en/latest/d32/d32_pro.html), connected to a [Waveshare 7.5" with a e-Paper Driver HAT](https://www.waveshare.com/product/displays/e-paper/epaper-1/7.5inch-e-paper-hat.htm), powered by a 3.7v 1800 mAh LiPo battery.

[^1]: Well, not really. It updates on a regular basis during the day.

# Credits

Inspired by [kristiantm](https://www.instructables.com/member/kristiantm/)'s [Instructable](https://www.instructables.com/E-Ink-Family-Calendar-Using-ESP32/) ([GitHub](https://github.com/kristiantm/eink-family-calendar-esp32)).

# Structure

The codebase is split into three directories.

## frontend

Frontend for user input.

## backend

Backend for the frontend and the esp32.

## esp32

The code running on the esp32.

# Attribution

Weather data from [MET Norway](https://api.met.no/), licensed under
[NLOD 2.0](https://data.norge.no/nlod/en/2.0) and
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).

The weather icons in `esp32/src/met_icons_black_50x50.cpp` come from
[met-bitmap-icons](https://github.com/engvik/met-bitmap-icons) and are derived
from the MET Norway / Yr weather symbols, licensed under the MIT License,
copyright (c) 2015-2017 Yr.
