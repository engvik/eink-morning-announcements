#ifndef CONFIG_H
#define CONFIG_H

// Deep sleep

#define uS_TO_S_FACTOR 1000000ULL
extern int SLEEP_TIME;
extern int LONG_SLEEP_TIME;
extern int LONG_SLEEP_HOUR;

// WiFi

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// Pins

extern int PIN_CS;
extern int PIN_DC;
extern int PIN_RST;
extern int PIN_BUSY;
extern int PIN_BATTERY;

// Eink Display

extern int X_DEFAULT_PADDING;
extern int Y_DEFAULT_PADDING;
extern int X_DEFAULT_SPACING;
extern int Y_DEFAULT_SPACING;

extern int BITMAP_SIZE;

extern const char* HEADER_MAIN;
extern const char* HEADER_MOTD;
extern const char* HEADER_CALENDAR;
extern const char* HEADER_WEATHER;

extern const char* ERROR_UPDATING;

extern const char* MSG_EMPTY_MOTD;
extern const char* MSG_EMPTY_CALENDAR;
extern const char* MSG_EMPTY_WEATHER;

extern int TEXT_CUTOFF_THRESHOLD;

// HTTP Backend

extern const char* BACKEND_CALENDAR_ENDPOINT;
extern const char* BACKEND_MESSAGE_ENDPOINT;
extern const char* BACKEND_META_ENDPOINT;
extern const char* BACKEND_WEATHER_ENDPOINT;
extern const char* BACKEND_AUTHORIZATION_HEADER;

#endif
