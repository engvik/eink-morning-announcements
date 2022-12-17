// Deep sleep

int SLEEP_TIME = 3600;       // 1 hour
int LONG_SLEEP_TIME = 21600; // 6 hours

// WiFi

const char* WIFI_SSID = "ssid";
const char* WIFI_PASSWORD = "password";

// Pins

int PIN_CS = 5;
int PIN_DC = 0;
int PIN_RST = 2;
int PIN_BUSY = 15;
int PIN_BATTERY = 35;

// Eink Display

int X_DEFAULT_PADDING = 25;
int Y_DEFAULT_PADDING = 20;
int X_DEFAULT_SPACING = 50;
int Y_DEFAULT_SPACING = 10;

int BITMAP_SIZE = 50;

const char* ERROR_UPDATING = "Unable to update :-(";

const char* MSG_EMPTY_MOTD = "No message today!";
const char* MSG_EMPTY_CALENDAR = "Nothing going on today!";
const char* MSG_EMPTY_WEATHER = "No weather reports today!";

int TEXT_CUTOFF_THRESHOLD = 39;

// HTTP Backend

const char* BACKEND_CALENDAR_ENDPOINT = "http://192.168.1.1:8080/api/calendar";
const char* BACKEND_MESSAGE_ENDPOINT = "http://192.168.1.1:8080/api/message";
const char* BACKEND_META_ENDPOINT = "http://192.168.1.1:8080/api/meta";
const char* BACKEND_WEATHER_ENDPOINT = "http://192.168.1.1:8080/api/weather";
const char* BACKEND_AUTHORIZATION_HEADER = "";
