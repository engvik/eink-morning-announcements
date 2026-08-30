#pragma once

#include <cstdint>

static_assert(sizeof(CFG_WIFI_SSID) > 1,
              "WIFI_SSID is empty, set it in the environment");
static_assert(sizeof(CFG_WIFI_PASSWORD) > 1,
              "WIFI_PASSWORD is empty, set it in the environment");
static_assert(sizeof(CFG_BACKEND_HOST) > 1,
              "BACKEND_HOST is empty, set it in the environment");

// Deep sleep

inline constexpr std::uint64_t uS_TO_S_FACTOR = 1000000ULL;

inline constexpr int SLEEP_TIME = 3600;        // 1 hour
inline constexpr int LONG_SLEEP_TIME = 21600;  // 6 hours
// Long sleep after the update at 23:00.
inline constexpr int LONG_SLEEP_HOUR = 23;

// WiFi

inline constexpr const char* WIFI_SSID = CFG_WIFI_SSID;
inline constexpr const char* WIFI_PASSWORD = CFG_WIFI_PASSWORD;

inline constexpr int WIFI_MAX_ATTEMPTS = 5;

// Pins

inline constexpr int PIN_CS = 5;
inline constexpr int PIN_DC = 0;
inline constexpr int PIN_RST = 2;
inline constexpr int PIN_BUSY = 15;
inline constexpr int PIN_BATTERY = 35;

// Battery

inline constexpr int BATTERY_SAMPLES = 16;
// 100k/100k divider on PIN_BATTERY
inline constexpr float BATTERY_DIVIDER_RATIO = 2.0f;

// Eink Display

inline constexpr int X_DEFAULT_PADDING = 25;
inline constexpr int Y_DEFAULT_PADDING = 20;
inline constexpr int X_DEFAULT_SPACING = 50;
inline constexpr int Y_DEFAULT_SPACING = 10;

inline constexpr int BITMAP_SIZE = 50;

inline constexpr const char* ERROR_UPDATING = "Unable to update :-(";

inline constexpr const char* MSG_EMPTY_MOTD = "No message today!";
inline constexpr const char* MSG_EMPTY_CALENDAR = "Nothing going on today!";
inline constexpr const char* MSG_EMPTY_WEATHER = "No weather reports today!";

inline constexpr int TEXT_CUTOFF_THRESHOLD = 39;

// HTTP Backend

inline constexpr const char* BACKEND_CALENDAR_ENDPOINT =
    CFG_BACKEND_HOST "/api/calendar";
inline constexpr const char* BACKEND_MESSAGE_ENDPOINT =
    CFG_BACKEND_HOST "/api/message";
inline constexpr const char* BACKEND_META_ENDPOINT =
    CFG_BACKEND_HOST "/api/meta";
inline constexpr const char* BACKEND_WEATHER_ENDPOINT =
    CFG_BACKEND_HOST "/api/weather";

// Optional, an empty token means no Authorization header is sent.
inline constexpr const char* BACKEND_AUTHORIZATION_HEADER = CFG_BACKEND_TOKEN;
