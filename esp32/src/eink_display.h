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

struct DisplayData {
    JSONVar calendar;
    JSONVar message;
    JSONVar meta;
    JSONVar weather;
};

String buildMainHeaderString(const char* weekday);
String buildTemperaturesString(String prefix, JSONVar weather);
String buildUpcomingWeatherString(String prefix, double precip, const char* symbol);
String buildLastUpdateString(const char* now);

class EinkDisplay {
    public:
        void init();
        void hibernate();
        void off();
        void refreshScreen();
        void draw(DisplayData *data);
};

#endif
