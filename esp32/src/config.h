#ifndef CONFIG_H
#define CONFIG_H

// WiFi

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// Eink Display

// TODO: Define display here?

extern int PIN_CS;
extern int PIN_DC;
extern int PIN_RST;
extern int PIN_BUSY;

extern int X_DEFAULT_PADDING;
extern int Y_DEFAULT_PADDING;
extern int X_DEFAULT_SPACING;
extern int Y_DEFAULT_SPACING;

extern const char* HEADER_MAIN;
extern const char* HEADER_MOTD;
extern const char* HEADER_CALENDAR;
extern const char* HEADER_WEATHER;

extern const char* ERROR_UPDATING;

extern const char* MSG_EMPTY_MOTD;
extern const char* MSG_EMPTY_CALENDAR;
extern const char* MSG_EMPTY_WEATHER;

// HTTP Backend

extern const char* BACKEND_CALENDAR_ENDPOINT;
extern const char* BACKEND_MESSAGE_ENDPOINT;
extern const char* BACKEND_META_ENDPOINT;
extern const char* BACKEND_WEATHER_ENDPOINT;

#endif
