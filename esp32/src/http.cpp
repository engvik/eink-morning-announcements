#include "http.h"

HttpResponse BackendClient::get(const char* url) {
    HttpResponse response;

    if (!http.begin(client, url)) {
        Serial.printf("\tUnable to start request for %s\n", url);
        return response;
    }

    // addHeader replaces an existing header of the same name by default, so
    // setting this every request does not accumulate.
    if (BACKEND_AUTHORIZATION_HEADER[0] != '\0') {
        http.addHeader("Authorization", BACKEND_AUTHORIZATION_HEADER);
    }

    response.status = http.GET();

    if (response.ok()) {
        response.body = http.getString();
        Serial.printf("\tHTTP %d, %u bytes\n", response.status, response.body.length());
    } else if (response.status < 0) {
        Serial.printf("\tTransport error %d: %s\n", response.status,
                      HTTPClient::errorToString(response.status).c_str());
    } else {
        Serial.printf("\tHTTP %d\n", response.status);
    }

    http.end();

    return response;
}
