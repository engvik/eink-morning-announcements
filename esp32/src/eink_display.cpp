#include "eink_display.h"

GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

void EinkDisplay::init() {
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
}

void EinkDisplay::hibernate() {
  display.hibernate();
}

void EinkDisplay::off() {
    display.powerOff();
}

void EinkDisplay::refreshScreen() {
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
  }
  while (display.nextPage());
}

void EinkDisplay::draw(DisplayData *data) {
  Serial.println("Drawing ...");
  
  // Size of text variables
  int16_t sx, sy; uint16_t sw, sh;

  // X, Y positions
  int x = X_DEFAULT_PADDING;
  int y = Y_DEFAULT_PADDING;

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    // Main header
    Serial.println("\tMain header ...");
    display.setFont(&FreeMonoBold18pt7b);

    const char* today = data->meta["today"];
    String mainHeader = buildMainHeaderString(today).c_str();

    display.setCursor(x, y);
    display.print(mainHeader);
    display.getTextBounds(mainHeader, x, y, &sx, &sy, &sw, &sh);

    y = y  + sh + Y_DEFAULT_SPACING;

    // MOTD header
    Serial.println("\tMOTD header ...");
    display.setFont(&FreeMonoBold12pt7b);

    display.setCursor(x, y);
    display.print(HEADER_MOTD);
    display.getTextBounds(HEADER_MOTD, x, y, &sx, &sy, &sw, &sh);

    y = y + sh + Y_DEFAULT_SPACING;

    // MOTD
    Serial.println("\tMOTD ...");
    display.setFont(&FreeMonoBold9pt7b);

    display.setCursor(x, y);

    if (data->message.hasOwnProperty("message")) {
        const char* message = data->message["message"];
        if (strlen(message) == 0) {
            message = MSG_EMPTY_MOTD;
        }

        display.print(message);
        display.getTextBounds(message, x, y, &sx, &sy, &sw, &sh);
    } else {
        display.print(ERROR_UPDATING);
        display.getTextBounds(ERROR_UPDATING, x, y, &sx, &sy, &sw, &sh);
    }
    
    y = y + sh + Y_DEFAULT_SPACING;

     // Calendar
    Serial.println("\tCalendar header ...");
    display.setFont(&FreeMonoBold12pt7b);

    display.setCursor(x, y);
    display.print(HEADER_CALENDAR);
    display.getTextBounds(HEADER_CALENDAR, x, y, &sx, &sy, &sw, &sh);

    y = y + sh + Y_DEFAULT_SPACING;

    // Calendar events
    Serial.println("\tCalendar events ...");
    display.setFont(&FreeMonoBold9pt7b);

    if (JSON.typeof(data->calendar) == "array") {
        if (data->calendar.length() > 0) {
             for (int i = 0; i < data->calendar.length(); i++) {
                display.setCursor(x, y);

                const char* title = data->calendar[i]["title"];

                display.print(title);
                display.getTextBounds(title, x, y, &sx, &sy, &sw, &sh);
    
                // TODO: Figure out placements ..
                y = y + sh + Y_DEFAULT_SPACING;
             }
        } else {
            display.setCursor(x, y);
            display.print(MSG_EMPTY_CALENDAR);
            display.getTextBounds(MSG_EMPTY_CALENDAR, x, y, &sx, &sy, &sw, &sh);

            y = y + sh + Y_DEFAULT_SPACING;
        }
    } else {
        display.setCursor(x, y);
        display.print(ERROR_UPDATING);
        display.getTextBounds(ERROR_UPDATING, x, y, &sx, &sy, &sw, &sh);
        
        y = y + sh + Y_DEFAULT_SPACING;
    }

    // Weather
    Serial.println("\tWeather header ...");
    display.setFont(&FreeMonoBold12pt7b);

    display.setCursor(x, y);
    display.print(HEADER_WEATHER);
    display.getTextBounds(HEADER_WEATHER, x, y, &sx, &sy, &sw, &sh);

    y = y + sh + Y_DEFAULT_SPACING;

    // Weather data
    Serial.println("\tWeather events ...");
    display.setFont(&FreeMonoBold9pt7b);

    if (JSON.typeof(data->weather) == "array") {
        if (data->weather.length() > 0) {
            String temperaturesLine = buildTemperaturesString("Temperatures:", data->weather);

            display.setCursor(x, y);
            display.print(temperaturesLine);
            display.getTextBounds(temperaturesLine, x, y, &sx, &sy, &sw, &sh);

            y = y + sh + Y_DEFAULT_SPACING;

            JSONVar weather = data->weather[0];
            double precip = weather["six_hours"]["precipitation_amount"];
            String weatherLine = buildUpcomingWeatherString("Next 6 hours:", precip).c_str();

            display.setCursor(x, y);
            display.print(weatherLine);
            display.getTextBounds(weatherLine, x, y, &sx, &sy, &sw, &sh);

            y = y + sh + Y_DEFAULT_SPACING;

            const char* symbol = weather["six_hours"]["symbol_code"];

            if (symbol != "") {
                this->drawBitmap(symbol, x, y);
                y = y + 50 + Y_DEFAULT_SPACING;
            }

            precip = weather["twelve_hours"]["precipitation_amount"];
            weatherLine = buildUpcomingWeatherString("Next 12 hours:", precip).c_str();

            display.setCursor(x, y);
            display.print(weatherLine);
            display.getTextBounds(weatherLine, x, y, &sx, &sy, &sw, &sh);

            y = y + sh + Y_DEFAULT_SPACING;

            symbol = weather["twelve_hours"]["symbol_code"];
            if (symbol != "") {
                this->drawBitmap(symbol, x, y);
                y = y + 50 + Y_DEFAULT_SPACING;
            }
        } else {
            display.setCursor(x, y);
            display.print(MSG_EMPTY_WEATHER);
            display.getTextBounds(MSG_EMPTY_WEATHER, x, y, &sx, &sy, &sw, &sh);

            y = y + sh + Y_DEFAULT_SPACING;
        }
    } else {
        display.setCursor(x, y);
        display.print(ERROR_UPDATING);
        display.getTextBounds(ERROR_UPDATING, x, y, &sx, &sy, &sw, &sh);

        y = y + sh + Y_DEFAULT_SPACING;
    }


    // TODO: Place in lower right corner
    const char* now = data->meta["now"];
    String lastUpdated = buildLastUpdateString(now).c_str();

    display.setFont(&FreeMono9pt7b);
    display.setCursor(x, y);
    display.print(lastUpdated);
    display.getTextBounds(lastUpdated, x, y, &sx, &sy, &sw, &sh);
  }
  while (display.nextPage());
}

void EinkDisplay::drawBitmap(const char* icon, int x, int y) {
    const unsigned char* bitmap = getIcon(icon);
    if (bitmap == NULL) {
        Serial.println("Error: icon not found, " + String(icon));
        return;
    }

    display.drawBitmap(x, y, bitmap, 50, 50, GxEPD_BLACK);
}

String buildMainHeaderString(const char* weekday) {
    String prefixStr = String(HEADER_MAIN);

    if (strcmp(weekday, "") == 0) {
        return prefixStr + "!";
    }
    
    String weekdayStr = String(weekday);

    return prefixStr + ", it's " + weekdayStr + "!";
}

String buildTemperaturesString(String prefix, JSONVar weather) {
    String line = "";

    for (int i = 0; i < weather.length(); i++) {
        const char* timestamp = weather[i]["time"];
        double temp = weather[i]["instant"]["air_temperature"];
        String hourStr = String(timestamp).substring(11, 16);
        String tempStr = String(temp);
        line = line + hourStr + ":" + tempStr + "C, ";
    }

    return prefix + line;
}

String buildUpcomingWeatherString(String prefix, double precip) {
    String precipStr = String(precip);
    return prefix + " " + precipStr + " mm";
}

String buildLastUpdateString(const char* now) {
    String nowStr = String(now).substring(0, 19);

    return "Last update: " + nowStr;
}

const unsigned char* getIcon(const char* icon) {
    if (strcmp(icon, "snowshowers_night") == 0) return met_bitmap_black_50x50_snowshowers_night;
    else if (strcmp(icon, "heavyrain") == 0) return met_bitmap_black_50x50_heavyrain;
    else if (strcmp(icon, "heavyrainandthunder") == 0) return met_bitmap_black_50x50_heavyrainandthunder;
    else if (strcmp(icon, "sleet") == 0) return met_bitmap_black_50x50_sleet;
    else if (strcmp(icon, "snow") == 0) return met_bitmap_black_50x50_snow;
    else if (strcmp(icon, "snowandthunder") == 0) return met_bitmap_black_50x50_snowandthunder;
    else if (strcmp(icon, "fog") == 0) return met_bitmap_black_50x50_fog;
    else if (strcmp(icon, "sleetshowersandthunder_day") == 0) return met_bitmap_black_50x50_heavysleetshowersandthunder_day;
    else if (strcmp(icon, "sleetshowersandthunder_night") == 0) return met_bitmap_black_50x50_heavysleetshowersandthunder_night;
    else if (strcmp(icon, "snowshowersandthunder_day") == 0) return met_bitmap_black_50x50_heavysnowshowersandthunder_day;
    else if (strcmp(icon, "snowshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_heavysnowshowersandthunder_polartwilight;
    else if (strcmp(icon, "rainandthunder") == 0) return met_bitmap_black_50x50_heavyrainandthunder;
    else if (strcmp(icon, "sleetandthunder") == 0) return met_bitmap_black_50x50_heavysleetandthunder;
    else if (strcmp(icon, "lightrainshowersandthunder_day") == 0) return met_bitmap_black_50x50_lightrainshowers_day;
    else if (strcmp(icon, "lightrainshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_lightrainshowers_polartwilight;
    else if (strcmp(icon, "heavyrainshowersandthunder_day") == 0) return met_bitmap_black_50x50_heavyrainshowersandthunder_day;
    else if (strcmp(icon, "heavyrainshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_heavyrainshowersandthunder_polartwilight;
    else if (strcmp(icon, "lightssleetshowersandthunder_day") == 0) return met_bitmap_black_50x50_lightssleetshowersandthunder_day;
    else if (strcmp(icon, "lightssleetshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_lightssleetshowersandthunder_polartwilight;
    else if (strcmp(icon, "lightssleetshowersandthunder_night") == 0) return met_bitmap_black_50x50_lightssleetshowersandthunder_night;
    else if (strcmp(icon, "heavysleetshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_heavysleetshowersandthunder_polartwilight;
    else if (strcmp(icon, "heavysleetshowersandthunder_night") == 0) return met_bitmap_black_50x50_heavysleetshowersandthunder_night;
    else if (strcmp(icon, "lightssnowshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_lightssnowshowersandthunder_polartwilight;
    else if (strcmp(icon, "lightssnowshowersandthunder_night") == 0) return met_bitmap_black_50x50_lightssnowshowersandthunder_day;
    else if (strcmp(icon, "heavysnowshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_heavysnowshowersandthunder_polartwilight;
    else if (strcmp(icon, "heavysnowshowersandthunder_night") == 0) return met_bitmap_black_50x50_heavysnowshowersandthunder_night;
    else if (strcmp(icon, "lightsleetandthunder") == 0) return met_bitmap_black_50x50_lightsleetandthunder;
    else if (strcmp(icon, "heavysleetandthunder") == 0) return met_bitmap_black_50x50_heavysleetandthunder;
    else if (strcmp(icon, "lightsnowandthunder") == 0) return met_bitmap_black_50x50_lightsnowandthunder;
    else if (strcmp(icon, "heavysnowandthunder") == 0) return met_bitmap_black_50x50_heavysnowandthunder;
    else if (strcmp(icon, "lightrainshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_lightrainshowers_polartwilight;
    else if (strcmp(icon, "lightrainshowers_night") == 0) return met_bitmap_black_50x50_lightrainshowers_night;
    else if (strcmp(icon, "heavyrainshowers_day") == 0) return met_bitmap_black_50x50_heavyrainshowers_day;
    else if (strcmp(icon, "heavyrainshowers_night") == 0) return met_bitmap_black_50x50_heavyrainshowers_night;
    else if (strcmp(icon, "lightsleetshowers_day") == 0) return met_bitmap_black_50x50_lightsleetshowers_day;
    else if (strcmp(icon, "heavysnowshowers_night") == 0) return met_bitmap_black_50x50_heavysnowshowers_night;
    else if (strcmp(icon, "lightsleet") == 0) return met_bitmap_black_50x50_lightsleet;
    else if (strcmp(icon, "heavysleet") == 0) return met_bitmap_black_50x50_heavysleet;
    else if (strcmp(icon, "lightsnow") == 0) return met_bitmap_black_50x50_lightsnow;
    else if (strcmp(icon, "heavysnow") == 0) return met_bitmap_black_50x50_heavysnow;
    else if (strcmp(icon, "clearsky_day") == 0) return met_bitmap_black_50x50_clearsky_day;
    else if (strcmp(icon, "clearsky_polartwilight") == 0) return met_bitmap_black_50x50_clearsky_polartwilight;
    else if (strcmp(icon, "clearsky_night") == 0) return met_bitmap_black_50x50_clearsky_night;
    else if (strcmp(icon, "fair_day") == 0) return met_bitmap_black_50x50_fair_day;
    else if (strcmp(icon, "fair_polartwilight") == 0) return met_bitmap_black_50x50_fair_polartwilight;
    else if (strcmp(icon, "fair_night") == 0) return met_bitmap_black_50x50_fair_night;
    else if (strcmp(icon, "partlycloudy_day") == 0) return met_bitmap_black_50x50_partlycloudy_day;
    else if (strcmp(icon, "partlycloudy_polartwilight") == 0) return met_bitmap_black_50x50_partlycloudy_polartwilight;
    else if (strcmp(icon, "partlycloudy_night") == 0) return met_bitmap_black_50x50_partlycloudy_night;
    else if (strcmp(icon, "cloudy") == 0) return met_bitmap_black_50x50_cloudy;
    else if (strcmp(icon, "rainshowers_day") == 0) return met_bitmap_black_50x50_rainshowers_day;
    else if (strcmp(icon, "rainshowers_polartwilight") == 0) return met_bitmap_black_50x50_rainshowers_polartwilight;
    else if (strcmp(icon, "rainshowers_night") == 0) return met_bitmap_black_50x50_rainshowers_night;
    else if (strcmp(icon, "rainshowersandthunder_day") == 0) return met_bitmap_black_50x50_heavyrainshowersandthunder_day;
    else if (strcmp(icon, "rainshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_rainshowersandthunder_polartwilight;
    else if (strcmp(icon, "rainshowersandthunder_night") == 0) return met_bitmap_black_50x50_rainshowersandthunder_night;
    else if (strcmp(icon, "sleetshowers_day") == 0) return met_bitmap_black_50x50_sleetshowers_day;
    else if (strcmp(icon, "sleetshowers_polartwilight") == 0) return met_bitmap_black_50x50_sleetshowersandthunder_polartwilight;
    else if (strcmp(icon, "sleetshowers_night") == 0) return met_bitmap_black_50x50_sleetshowers_night;
    else if (strcmp(icon, "snowshowers_day") == 0) return met_bitmap_black_50x50_snowshowers_day;
    else if (strcmp(icon, "snowshowers_polartwilight") == 0) return met_bitmap_black_50x50_snowshowers_polartwilight;
    else if (strcmp(icon, "rain") == 0) return met_bitmap_black_50x50_rain;
    else if (strcmp(icon, "sleetshowersandthunder_polartwilight") == 0) return met_bitmap_black_50x50_sleetshowersandthunder_polartwilight;
    else if (strcmp(icon, "snowshowersandthunder_night") == 0) return met_bitmap_black_50x50_snowshowersandthunder_night;
    else if (strcmp(icon, "lightrainshowers_night") == 0) return met_bitmap_black_50x50_lightrainshowers_night;
    else if (strcmp(icon, "heavyrainshowersandthunder_night") == 0) return met_bitmap_black_50x50_heavyrainshowersandthunder_night;
    else if (strcmp(icon, "heavysleetshowersandthunder_day") == 0) return met_bitmap_black_50x50_heavysleetshowersandthunder_day;
    else if (strcmp(icon, "lightssnowshowersandthunder_day") == 0) return met_bitmap_black_50x50_lightssnowshowersandthunder_day;
    else if (strcmp(icon, "heavysnowshowersandthunder_day") == 0) return met_bitmap_black_50x50_heavysnowshowersandthunder_day;
    else if (strcmp(icon, "lightrainandthunder") == 0) return met_bitmap_black_50x50_lightrainandthunder;
    else if (strcmp(icon, "lightrainshowers_day") == 0) return met_bitmap_black_50x50_lightrainshowers_day;
    else if (strcmp(icon, "heavyrainshowers_polartwilight") == 0) return met_bitmap_black_50x50_heavyrainshowers_polartwilight;
    else if (strcmp(icon, "lightsleetshowers_polartwilight") == 0) return met_bitmap_black_50x50_lightsleetshowers_polartwilight;
    else if (strcmp(icon, "lightsleetshowers_night") == 0) return met_bitmap_black_50x50_lightsleetshowers_night;
    else if (strcmp(icon, "heavysleetshowers_day") == 0) return met_bitmap_black_50x50_heavysleetshowers_day;
    else if (strcmp(icon, "heavysleetshowers_polartwilight") == 0) return met_bitmap_black_50x50_heavysleetshowers_polartwilight;
    else if (strcmp(icon, "heavysleetshowers_night") == 0) return met_bitmap_black_50x50_heavysleetshowers_night;
    else if (strcmp(icon, "lightsnowshowers_day") == 0) return met_bitmap_black_50x50_lightsnowshowers_day;
    else if (strcmp(icon, "lightsnowshowers_polartwilight") == 0) return met_bitmap_black_50x50_lightsnowshowers_polartwilight;
    else if (strcmp(icon, "lightsnowshowers_night") == 0) return met_bitmap_black_50x50_lightssnowshowersandthunder_night;
    else if (strcmp(icon, "heavysnowshowers_day") == 0) return met_bitmap_black_50x50_heavysnowshowers_day;
    else if (strcmp(icon, "heavysnowshowers_polartwilight") == 0) return met_bitmap_black_50x50_heavysnowshowers_polartwilight;
    else if (strcmp(icon, "lightrain") == 0) return met_bitmap_black_50x50_lightrain;
    else return NULL;
}
