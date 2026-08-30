#ifndef HTTP_H
#define HTTP_H

#include <HTTPClient.h>
#include <WiFi.h>

#include "config.h"

struct HttpResponse {
  // 0 means no request sent, negative means transport error, positive means
  // HTTP
  int status = 0;
  String body;

  bool ok() const { return status == HTTP_CODE_OK; }
};

// BackendClient is a simple HTTP client that can send GET requests to a
// backend server.
class BackendClient {
 public:
  HttpResponse get(const char* url);

 private:
  WiFiClient client;
  HTTPClient http;
};

#endif
