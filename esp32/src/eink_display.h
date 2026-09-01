#pragma once

#include <Adafruit_GFX.h>
#include <Arduino_JSON.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxEPD2_BW.h>

#include "config.h"
#include "data.h"
#include "met_icons_black_50x50.h"

String buildMainHeaderString(JSONVar meta);
String buildTemperatureHourString(const char* timestamp, double temp);
String buildPrecipString(double precip);
String buildLastUpdateString(const char* now);
const unsigned char* getIcon(const char* icon);

class EinkDisplay {
 private:
  int x;
  int y;
  int width;
  int height;
  int16_t sx;
  int16_t sy;
  uint16_t sw;
  uint16_t sh;

  void drawMainHeader(JSONVar meta);
  void drawMOTD(JSONVar motd);
  void drawCalendar(JSONVar calendar, JSONVar meta);
  void drawWeather(JSONVar weather);
  void drawBattery(float voltage);
  void drawLastUpdated(JSONVar meta);

  int drawUpcomingWeather(JSONVar weather);
  void drawUpcomingWeatherPeriod(const char* period, const char* symbol,
                                 double precip);

  void drawText(const char* text);
  void drawBitmap(const char* icon);
  void setNextCursorPosition(int x, int y);

 public:
  void init();
  void hibernate();
  void refreshScreen();
  void draw(DisplayData* data);
};
