# eink-morning-announcements

Morning announcements on a eink display! Calendar, weather and custom messages.

This is built with a [Lolin D32 Pro](https://www.wemos.cc/en/latest/d32/d32_pro.html), connected to a [Waveshare 7.5" with a e-Paper Driver HAT](https://www.waveshare.com/product/displays/e-paper/epaper-1/7.5inch-e-paper-hat.htm), powered by a 3.7v 1800 mAh LiPo battery.

# Credits

Inspired by [kristiantm](https://www.instructables.com/member/kristiantm/)'s [Instructable](https://www.instructables.com/E-Ink-Family-Calendar-Using-ESP32/) ([GitHub](https://github.com/kristiantm/eink-family-calendar-esp32)).

#  Structure

The codebase is split into three directories.

## frontend

Frontend for user input.

## backend

Backend for the frontend and the esp32.

## esp32

The code running on the esp32.
