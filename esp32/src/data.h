#pragma once

#include <Arduino_JSON.h>

#include "http.h"

struct DisplayData {
  JSONVar calendar;
  JSONVar message;
  JSONVar meta;
  JSONVar weather;
  float battery = 0.0f;
};

DisplayData fetchDisplayData(BackendClient& backend);
