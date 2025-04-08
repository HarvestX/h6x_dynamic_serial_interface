#ifndef RS485_COMM_HPP
#define RS485_COMM_HPP

#include <Arduino.h>
#include "crc8.hpp"

constexpr int RS485_CTRL_PIN = 12;     // DE/REピン
constexpr int RS485_RX_PIN = 14;
constexpr int RS485_TX_PIN = 13;
constexpr int MAX_BUFFER_SIZE = 64;    // 受信バッファサイズ

inline void rs485_init() {
  Serial2.begin(9600, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  pinMode(RS485_CTRL_PIN, OUTPUT);
  digitalWrite(RS485_CTRL_PIN, LOW);  // 初期は受信モード
}

// RS485パケット送信（データと長さ）
inline void rs485_send_packet(const uint8_t* data, uint16_t len) {
  uint8_t crc = crc8_calculate(data, len);

  digitalWrite(RS485_CTRL_PIN, HIGH);  // 送信モード
  delay(1);

  Serial2.write(data, len);
  Serial2.write(crc);

  Serial2.flush();
  delay(1);
  digitalWrite(RS485_CTRL_PIN, LOW);   // 受信モード
}

// RS485パケット受信（受信バッファとデータ長を返す）
inline bool rs485_receive_packet(uint8_t* buffer, uint16_t* len) {
  if (Serial2.available() > 0) {
    uint16_t i = 0;
    while (Serial2.available() && i < MAX_BUFFER_SIZE - 1) {
      buffer[i++] = Serial2.read();
      delay(2);
    }

    if (i < 2) return false;

    uint8_t received_crc = buffer[i - 1];
    *len = i - 1;

    return crc8_calculate(buffer, *len) == received_crc;
  }
  return false;
}

#endif  // RS485_COMM_HPP
