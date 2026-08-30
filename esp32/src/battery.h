#pragma once

#include <Arduino.h>

#include "config.h"

float getBatteryVoltage();
int getBatteryPercentage(float voltage);
