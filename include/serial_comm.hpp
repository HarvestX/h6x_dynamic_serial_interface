#ifndef SERIAL_COMM_HPP
#define SERIAL_COMM_HPP

#include <Arduino.h>
#include "crc8.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PAYLOAD_SIZE 245
#define HEADER_BYTE 0x23  // '#'
#define FOOTER_BYTE 0x0D  // '\r'

static inline void send_structured_packet(uint8_t device_id, uint8_t target_id, uint8_t command, const uint8_t* data, uint8_t data_len) {
    if (data_len > MAX_PAYLOAD_SIZE) return;

    uint8_t total_len = 6 + data_len;
    uint8_t tx_buf[256];  // MAX 250Byte

    tx_buf[0] = HEADER_BYTE;
    tx_buf[1] = device_id;
    tx_buf[2] = target_id;
    tx_buf[3] = command;

    if (data_len > 0 && data != NULL) {
        memcpy(&tx_buf[4], data, data_len);
    }

    uint8_t crc = crc8_calculate(&tx_buf[1], 3 + data_len);
    tx_buf[4 + data_len] = crc;
    tx_buf[5 + data_len] = FOOTER_BYTE;

    Serial.write(tx_buf, total_len);
}

#ifdef __cplusplus
}
#endif

#endif // SERIAL_COMM_HPP
