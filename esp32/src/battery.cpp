#include "battery.h"

float getBatteryVoltage() {
    return analogRead(PIN_BATTERY) / 4096.0 * 7.445;
}
