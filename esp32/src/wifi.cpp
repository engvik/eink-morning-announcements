#include "wifi.h"


const char* ssid = "ssid";
const char* passord = "password";

void initWiFi() {
    Serial.println("Setting up WiFi ..");

    WiFi.mode(WIFI_STA);
    WiFi.begin("", "");

    Serial.print("\tConnecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }

    Serial.println("\tConnected with IP: " + WiFi.localIP());
}
