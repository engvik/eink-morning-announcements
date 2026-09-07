// A 1-bit greyscale PNG writer, so a GFXcanvas1 can be saved without pulling
// in zlib. Deflate "stored" blocks are uncompressed, which keeps this small at
// the cost of a file a few tens of kilobytes larger than it needs to be.
#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace png {

inline uint32_t crc32(const uint8_t *data, size_t length, uint32_t crc = 0) {
  static uint32_t table[256];
  static bool ready = false;

  if (!ready) {
    for (uint32_t i = 0; i < 256; i++) {
      uint32_t c = i;

      for (int k = 0; k < 8; k++) {
        c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
      }

      table[i] = c;
    }

    ready = true;
  }

  crc = ~crc;

  for (size_t i = 0; i < length; i++) {
    crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  }

  return ~crc;
}

inline void appendBE32(std::vector<uint8_t> &out, uint32_t value) {
  out.push_back(value >> 24);
  out.push_back(value >> 16);
  out.push_back(value >> 8);
  out.push_back(value);
}

inline void appendChunk(std::vector<uint8_t> &out, const char *type,
                        const std::vector<uint8_t> &payload) {
  appendBE32(out, static_cast<uint32_t>(payload.size()));

  std::vector<uint8_t> body(type, type + 4);
  body.insert(body.end(), payload.begin(), payload.end());

  out.insert(out.end(), body.begin(), body.end());
  appendBE32(out, crc32(body.data(), body.size()));
}

// write saves a packed 1-bit-per-pixel buffer, MSB first, as GFXcanvas1 holds
// it. A set bit means ink, which PNG greyscale renders as 0, so bits are
// inverted on the way out.
inline bool write(const std::string &path, const uint8_t *bits, uint16_t width,
                  uint16_t height) {
  const size_t stride = (width + 7) / 8;

  // Raw scanlines: a filter byte per row, then the row itself.
  std::vector<uint8_t> raw;
  raw.reserve(height * (stride + 1));

  for (uint16_t y = 0; y < height; y++) {
    raw.push_back(0);  // filter: none

    for (size_t x = 0; x < stride; x++) {
      raw.push_back(~bits[y * stride + x]);
    }
  }

  // zlib stream: header, stored deflate blocks, adler32 of the raw data.
  std::vector<uint8_t> zlib{0x78, 0x01};

  for (size_t offset = 0; offset < raw.size();) {
    const uint16_t block =
        static_cast<uint16_t>(std::min<size_t>(65535, raw.size() - offset));
    const bool last = offset + block >= raw.size();

    zlib.push_back(last ? 1 : 0);
    zlib.push_back(block & 0xFF);
    zlib.push_back(block >> 8);
    zlib.push_back(~block & 0xFF);
    zlib.push_back((~block >> 8) & 0xFF);
    zlib.insert(zlib.end(), raw.begin() + offset, raw.begin() + offset + block);

    offset += block;
  }

  uint32_t a = 1, b = 0;
  for (uint8_t byte : raw) {
    a = (a + byte) % 65521;
    b = (b + a) % 65521;
  }
  appendBE32(zlib, (b << 16) | a);

  std::vector<uint8_t> out{0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

  std::vector<uint8_t> ihdr;
  appendBE32(ihdr, width);
  appendBE32(ihdr, height);
  ihdr.push_back(1);  // bit depth
  ihdr.push_back(0);  // colour type: greyscale
  ihdr.push_back(0);  // compression
  ihdr.push_back(0);  // filter
  ihdr.push_back(0);  // interlace

  appendChunk(out, "IHDR", ihdr);
  appendChunk(out, "IDAT", zlib);
  appendChunk(out, "IEND", {});

  FILE *file = fopen(path.c_str(), "wb");
  if (file == nullptr) {
    return false;
  }

  const bool ok = fwrite(out.data(), 1, out.size(), file) == out.size();
  fclose(file);

  return ok;
}

}  // namespace png
