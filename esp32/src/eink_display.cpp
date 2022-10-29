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
            const char* symbol = weather["six_hours"]["symbol_code"];
            String weatherLine = buildUpcomingWeatherString("Next 6 hours:", precip, symbol).c_str();

            display.setCursor(x, y);
            display.print(weatherLine);
            display.getTextBounds(weatherLine, x, y, &sx, &sy, &sw, &sh);

            y = y + sh + Y_DEFAULT_SPACING;

            precip = weather["twelve_hours"]["precipitation_amount"];
            symbol = weather["twelve_hours"]["symbol_code"];
            weatherLine = buildUpcomingWeatherString("Next 12 hours:", precip, symbol).c_str();

            display.setCursor(x, y);
            display.print(weatherLine);
            display.getTextBounds(weatherLine, x, y, &sx, &sy, &sw, &sh);

            y = y + sh + Y_DEFAULT_SPACING;
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

String buildUpcomingWeatherString(String prefix, double precip, const char* symbol) {
    String precipStr = String(precip);
    String symbolStr = String(symbol);
    return prefix + " " + precipStr + " mm, " + symbolStr;
}

String buildLastUpdateString(const char* now) {
    String nowStr = String(now).substring(0, 19);

    return "Last update: " + nowStr;
}
