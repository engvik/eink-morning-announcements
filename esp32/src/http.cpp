#include "http.h"

String httpGET(const char* serverName) {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);

    if (BACKEND_AUTHORIZATION_HEADER != "") {
        http.addHeader("Authorization", BACKEND_AUTHORIZATION_HEADER);
    }

    int httpStatusCode = http.GET();

    String payload = "{}";

    if (httpStatusCode == 200) {
        Serial.print("\tHTTP Status code: ");
        Serial.println(httpStatusCode);
        payload = http.getString();
    } else {
        Serial.print("\tError code: ");
        Serial.println(httpStatusCode);
    }

    http.end();

    return payload;
}

