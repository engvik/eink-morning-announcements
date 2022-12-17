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
  
  // X, Y positions
  this->x = 0;
  this->y = Y_DEFAULT_PADDING * 3;

  // Display width and height
  this->width = display.width();
  this->height = display.height();

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    this->drawMainHeader(data->meta);
    display.drawFastHLine(0, this->y, this->width - (X_DEFAULT_SPACING), GxEPD_BLACK);
    this->setNextCursorPosition(0, this->y + this->sh + Y_DEFAULT_SPACING);

    this->drawMOTD(data->message);
    display.drawFastHLine(0, this->y, this->width - (X_DEFAULT_PADDING*2), GxEPD_BLACK);
    this->setNextCursorPosition(this->x, this->y + (Y_DEFAULT_SPACING*3)); // Add padding

    this->drawCalendar(data->calendar, data->meta);
    display.drawFastHLine(0, this->y, this->width - (X_DEFAULT_PADDING*2), GxEPD_BLACK);
    this->setNextCursorPosition(this->x, this->y + this->sh + (Y_DEFAULT_SPACING*2)); // Add padding

    this->drawWeather(data->weather);
    display.drawFastHLine(0, this->y + Y_DEFAULT_PADDING, this->width - (X_DEFAULT_PADDING*2), GxEPD_BLACK);
    this->setNextCursorPosition(this->x, this->y + this->sh + (Y_DEFAULT_SPACING*2)); // Add padding

    display.setFont(&FreeMono9pt7b);

    this->drawBattery(data->battery);
    this->drawLastUpdated(data->meta);
  }
  while (display.nextPage());
}

/**
 * Draws the main header.
 */
void EinkDisplay::drawMainHeader(JSONVar meta) {
    // Main header
    Serial.println("\tMain header ...");

    String mainHeader = buildMainHeaderString(meta);

    display.setFont(&FreeMonoBold12pt7b);
    this->drawText(mainHeader.c_str());
    this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);
}

/**
 * Draws the message of the day.
 */
void EinkDisplay::drawMOTD(JSONVar motd) {
    Serial.println("\tMOTD ...");
    display.setFont(&FreeMonoBold9pt7b);

    if (motd.hasOwnProperty("message")) {
        const char* message = motd["message"];

        if (strlen(message) == 0) {
            message = MSG_EMPTY_MOTD;
        }

        this->drawText(message);
    } else {
        this->drawText(ERROR_UPDATING);
    }
    
    this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);

}

/**
 * Draws the calendar events.
 */
void EinkDisplay::drawCalendar(JSONVar calendar, JSONVar meta) {
    Serial.println("\tCalendar events ...");
    display.setFont(&FreeMonoBold9pt7b);

    if (JSON.typeof(calendar) == "array") {
        if (calendar.length() > 0) {
            String prevDay = "";

             for (int i = 0; i < calendar.length(); i++) {
                const char* title = calendar[i]["title"];
                const char* location = calendar[i]["location"];
                const char* start = calendar[i]["start"];
                const char* end = calendar[i]["end"];

                String titleStr = String(title);
                String locationStr = String(location);
                String startDate = String(start).substring(0, 10);
                String startHour = String(start).substring(11, 16);

                String event = "";

                const char* startDay = meta["date_to_weekday"][startDate];
                String startDayStr = String(startDay);

                if (prevDay != startDayStr && startDayStr != "") {
                    this->drawText(startDayStr.c_str());
                    this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);
                }
                
                prevDay = startDayStr;

                event = startHour + ": " + titleStr;
               
                if (locationStr != "") {
                    event = event + " (" + location + ")";
                }

                this->drawText(event.c_str());
                this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);
             }
        } else {
            this->drawText(MSG_EMPTY_CALENDAR);
            this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);
        }
    } else {
        this->drawText(ERROR_UPDATING);
        this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);
    }
}

/**
 * Draws the weather data.
 */
void EinkDisplay::drawWeather(JSONVar weather) {
    Serial.println("\tWeather data ...");
    display.setFont(&FreeMonoBold9pt7b);

    if (JSON.typeof(weather) == "array") {
        if (weather.length() > 0) {
            int secondColumnYPos = this->y;

            // Left column
            int textWidth = this->drawUpcomingWeather(weather);
          
            // Set cursor at right column
            this->setNextCursorPosition(this->x + textWidth + X_DEFAULT_SPACING, secondColumnYPos);

            int secondColumnXPos = this->x;
    
            // Right column
            const char* symbol = weather[0]["one_hour"]["symbol_code"];
            double precip = weather[0]["one_hour"]["precipitation_amount"];
            this->drawUpcomingWeatherPeriod("Next hour", symbol, precip);

            this->setNextCursorPosition(secondColumnXPos, this->y);

            symbol = weather[0]["six_hours"]["symbol_code"];
            precip = weather[0]["six_hours"]["precipitation_amount"];
            this->drawUpcomingWeatherPeriod("Next 6 hours", symbol, precip);

            this->setNextCursorPosition(secondColumnXPos, this->y);
          
            symbol = weather[0]["twelve_hours"]["symbol_code"];
            precip = weather[0]["twelve_hours"]["precipitation_amount"];
            this->drawUpcomingWeatherPeriod("Next 12 hours", symbol, precip);

            // Reset X position
            this->setNextCursorPosition(X_DEFAULT_PADDING, this->y);
        } else {
            this->drawBitmap(MSG_EMPTY_WEATHER);
            this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);
        }
    } else {
        this->drawBitmap(ERROR_UPDATING);
        this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);
    }
}


/**
 * Draw the battery level. Calculates percentage based on the current battery
 * voltage.
 */
void EinkDisplay::drawBattery(float voltage) {
    Serial.println("\tBattery ...");

    int percentage = -1;

    if (voltage >= 4.1) percentage = 100;
    else if (voltage >= 4.0) percentage = 90;
    else if (voltage >= 3.9) percentage = 80;
    else if (voltage >= 3.8) percentage = 75;
    else if (voltage >= 3.7) percentage = 60;
    else if (voltage >= 3.6) percentage = 50;
    else if (voltage >= 3.5) percentage = 35;
    else if (voltage >= 3.4) percentage = 25;
    else if (voltage >= 3.3) percentage = 10;
    else if (voltage <= 3.2) percentage = 0;

    String batteryStr = String(percentage) + "%";

    this->setNextCursorPosition(0, this->height - (Y_DEFAULT_PADDING*5));

    this->drawText(batteryStr.c_str());
}

/**
 * Draws the last updated information in the lower right corner.
 *
 * Gets the size of the text first and uses it's size with the display width
 * and height to calculate the position.
 */
void EinkDisplay::drawLastUpdated(JSONVar meta) {
    Serial.println("\tLast updated ...");

    const char* now = meta["now"];
    String lastUpdated = buildLastUpdateString(now);

    display.getTextBounds(lastUpdated.c_str(), this->x, this->y, &this->sx, &this->sy, &this->sw, &this->sh);
    this->setNextCursorPosition(this->width - this->sw - (X_DEFAULT_PADDING*2), this->y);
    this->drawText(lastUpdated.c_str());
}

/**
 * Draws the upcoming temepratures and precipitation if above 0. Returns the
 * widest text width seen. This is used for calculating the placement of the
 * second column, so nothing overlaps.
 */
int EinkDisplay::drawUpcomingWeather(JSONVar weather) {
    int textWidth = 0;

    for (int i = 0; i < weather.length(); i++) {
        const char* timestamp = weather[i]["time"];
        double temp = weather[i]["instant"]["air_temperature"];
        double precip = weather[i]["one_hour"]["precipitation_amount"];
        String line = buildTemperatureHourString(timestamp, temp);
      
        if (precip > 0.00) {
            String precipStr = buildPrecipString(precip);
            line = line + ", " + precipStr;
        }
                 
        this->drawText(line.c_str());
        this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);

        if (textWidth < this->sw) {
            textWidth = this->sw;
        }
    }

    return textWidth;
}

/**
 * Draws the weather symbol and precipitation for the next period.
 */
void EinkDisplay::drawUpcomingWeatherPeriod(const char* period, const char* symbol, double precip) {
    this->drawText(period);
    this->setNextCursorPosition(this->x, this->y + this->sh);

    if (symbol != "") {
        this->drawBitmap(symbol);
        this->setNextCursorPosition(this->x + BITMAP_SIZE + (X_DEFAULT_SPACING / 3), this->y + (BITMAP_SIZE / 2));
    }

    String precipStr = buildPrecipString(precip);
    this->drawText(precipStr.c_str());
    this->setNextCursorPosition(this->x, this->y + (BITMAP_SIZE / 2) + Y_DEFAULT_SPACING);
}

/**
 * Draws text to the display. If the current text fills more than the width of
 * the display, split it into multiple lines.
 */
void EinkDisplay::drawText(const char* text) {
    display.getTextBounds(text, this->x, this->y, &this->sx, &this->sy, &this->sw, &this->sh);
    
    if ((this->sx + this->sw + X_DEFAULT_SPACING) > this->width) {
        String tmp = String(text);
        int textWidth = tmp.length();

        // Keep splitting the text into multiple lines as long as it's above
        // the cutoff threshold. To make things simpler, user the Arduino
        // String as there is no need to optimize that much.
        while (textWidth > TEXT_CUTOFF_THRESHOLD) {
            int splitPoint = tmp.substring(0, TEXT_CUTOFF_THRESHOLD).lastIndexOf(' ');

            String splitText = tmp.substring(0, splitPoint);

            display.setCursor(this->x, this->y);
            display.print(splitText.c_str());
            display.getTextBounds(splitText, this->x, this->y, &this->sx, &this->sy, &this->sw, &this->sh);
            this->setNextCursorPosition(this->x, this->y + this->sh + Y_DEFAULT_SPACING);
            
            tmp = tmp.substring(splitPoint+1, textWidth);
            textWidth = tmp.length();
        }

        // Draw the rest of the string regularily.
        text = tmp.c_str();
    }

    display.setCursor(this->x, this->y);
    display.print(text);
}

/**
 * Draws a bitmap icon to the current cursor position.
 */
void EinkDisplay::drawBitmap(const char* icon) {
    const unsigned char* bitmap = getIcon(icon);
    if (bitmap == NULL) {
        Serial.println("Error: icon not found, " + String(icon));
        return;
    }

    display.drawBitmap(this->x, this->y, bitmap, BITMAP_SIZE, BITMAP_SIZE, GxEPD_BLACK);
}

/**
 * Updates x and y coordinates.
 */
void EinkDisplay::setNextCursorPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

String buildMainHeaderString(JSONVar meta) {
    const char* today = meta["today"];
    const char* month = meta["month"];
    int date = meta["date"];
    int week = meta["week"];
    int year = meta["year"];
    
    String weekdayStr = String(today);
    String dateStr = String(date);
    String weekStr = String(week);
    String monthStr = String(month);
    String yearStr = String(year);

    return weekdayStr + " " + dateStr + " " + monthStr + " " + yearStr + " (W " + weekStr + ")";
}

String buildTemperatureHourString(const char* timestamp, double temp) {
    String hourStr = String(timestamp).substring(11, 16);
    String tempStr = String(temp);

    return hourStr + ": " + tempStr + "C";
}

String buildPrecipString(double precip) {
    String precipStr = String(precip);
    return precipStr + " mm";
}

String buildLastUpdateString(const char* now) {
    String nowStr = String(now).substring(0, 19);

    return nowStr;
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
    else if (strcmp(icon, "lightrainshowersandthunder_day") == 0) return met_bitmap_black_50x50_lightrainshowersandthunder_day;
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
