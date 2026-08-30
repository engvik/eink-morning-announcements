#include "battery.h"

struct BatteryLevel {
  float voltage;
  int percentage;
};

constexpr BatteryLevel BATTERY_CURVE[] = {
    {4.1f, 100}, {4.0f, 90}, {3.9f, 80}, {3.8f, 75}, {3.7f, 60},
    {3.6f, 50},  {3.5f, 35}, {3.4f, 25}, {3.3f, 10},
};

float getBatteryVoltage() {
  std::uint32_t total = 0;

  for (int i = 0; i < BATTERY_SAMPLES; i++) {
    total += analogReadMilliVolts(PIN_BATTERY);
  }

  const float millivolts = static_cast<float>(total) / BATTERY_SAMPLES;

  return millivolts * BATTERY_DIVIDER_RATIO / 1000.0f;
}

int getBatteryPercentage(float voltage) {
  for (const BatteryLevel& level : BATTERY_CURVE) {
    if (voltage >= level.voltage) {
      return level.percentage;
    }
  }

  return 0;
}
