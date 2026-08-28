#include "wifi.h"

bool initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("\tConnecting to WiFi ..");

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_ATTEMPTS) {
        Serial.print('.');
        delay(1000);
        attempts++;
    }

    Serial.print('\n');

    return WiFi.status() == WL_CONNECTED;
}
