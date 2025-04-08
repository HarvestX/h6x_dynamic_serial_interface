#ifndef CRC8_HPP
#define CRC8_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint8_t crc8_calculate(const uint8_t *data, uint16_t len) {
  uint8_t crc = 0;

  for (uint16_t i = 0; i < len; i++) {
      uint8_t extract = data[i];
      for (uint8_t j = 8; j > 0; j--) {
          uint8_t sum = (crc ^ extract) & 0x01;
          crc >>= 1;
          if (sum) {
              crc ^= 0x8C; // CRC-8 多項式: x^8 + x^5 + x^4 + 1 (0x8C)
          }
          extract >>= 1;
      }
  }

  return crc;
}

#ifdef __cplusplus
}
#endif

#endif // CRC8_HPP
