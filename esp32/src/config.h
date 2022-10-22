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

// HTTP Backend

extern const char* BACKEND_CALENDAR_ENDPOINT;
extern const char* BACKEND_MESSAGE_ENDPOINT;
extern const char* BACKEND_WEATHER_ENDPOINT;

#endif
