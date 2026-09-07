// Minimal stand-in for Arduino's Print, which Adafruit_GFX derives from.
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

class Print {
 public:
  virtual ~Print() = default;

  virtual size_t write(uint8_t c) = 0;

  virtual size_t write(const uint8_t *buffer, size_t size) {
    size_t written = 0;

    while (size--) {
      if (write(*buffer++) == 0) {
        break;
      }

      written++;
    }

    return written;
  }

  size_t print(const char *s) {
    return s ? write(reinterpret_cast<const uint8_t *>(s), strlen(s)) : 0;
  }

  size_t print(char c) { return write(static_cast<uint8_t>(c)); }
};
