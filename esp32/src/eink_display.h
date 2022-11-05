#ifndef EINK_DISPLAY_H
#define EINK_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Arduino_JSON.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <GxEPD2_BW.h>

#include "config.h"
#include "met_icons_black_50x50.cpp"

struct DisplayData {
    JSONVar calendar;
    JSONVar message;
    JSONVar meta;
    JSONVar weather;
};

String buildMainHeaderString(const char* weekday);
String buildTemperaturesString(String prefix, JSONVar weather);
String buildUpcomingWeatherString(String prefix, double precip);
String buildLastUpdateString(const char* now);
const unsigned char* getIcon(const char* icon);

class EinkDisplay {
    public:
        void init();
        void hibernate();
        void off();
        void refreshScreen();
        void draw(DisplayData *data);
        void drawBitmap(const char* icon, int x, int y);
};

#endif
