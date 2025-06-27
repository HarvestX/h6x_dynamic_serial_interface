#ifndef SERIAL_HANDLER_HPP
#define SERIAL_HANDLER_HPP

#include <M5Core2.h>
#include <string.h>
#include "protocol_definitions.h"
#include "h6x_dynamic_packet_crc8.h"
#include "command_handler.hpp"
#include "packet_handler.h"
#include "cpu_usage_handler.hpp"


bool serial_read(Packet & pkt, const uint8_t mode, const uint8_t own_id)
{
  uint8_t header_error_count = 0;
  uint8_t header = 0;
  while (header_error_count < 100) {
    if (Serial1.available() > 0) {
      header = Serial1.read();  // Read packet header
      Serial.printf("%02X\n", header);
      if (header == '#' && mode == SERIAL_MODE_SECONDARY) {
        break;  // Valid header found
      } else if (header == '$' && mode == SERIAL_MODE_PRIMARY) {
        break;  // Valid header found
      } else {
        header_error_count++;
      }
    }
    delay(1);  // Wait for next byte
  }

  Serial.printf("\n");

  if (header_error_count >= 100) {
    M5.Lcd.setCursor(0, 30);
    M5.Lcd.printf("Header error\n");
    return false;  // Header not found
  }

  char data[256] = {header};
  int read_data_len = 1;
  int max_len = sizeof(data);

  while (read_data_len < max_len) {
    while (!Serial1.available()) {}
    int byte = Serial1.read();
    if (byte == -1) {continue;}

    data[read_data_len++] = static_cast<char>(byte);

    if (byte == '\r') {break;}
  }
  packet_division(&pkt, data, read_data_len);

  // Check CRC
  if (!check_crc(&pkt)) {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("CRC error\n");
    return false;  // CRC check failed
  }

  if (mode == SERIAL_MODE_SECONDARY && pkt.target_id != own_id) {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Invalid target ID\n");
    return false;  // Invalid target ID
  }

  // Debug print received data to M5 LCD
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("=== RECV ===\n");
  M5.Lcd.printf("header: %02X\n", pkt.header);
  M5.Lcd.printf("target_id: %02X\n", pkt.target_id);
  M5.Lcd.printf("command: %02X\n", pkt.command);
  M5.Lcd.printf("data_len: %02X\n", pkt.data_len);
  M5.Lcd.print("recv_data: ");
  for (int i = 0; i < pkt.data_len; ++i) {
    M5.Lcd.printf("%02X ", pkt.data[i]);
  }
  M5.Lcd.println();
  M5.Lcd.printf("crc: %02X\n", pkt.crc);
  M5.Lcd.printf("footer: %02X\n", pkt.footer);
  M5.Lcd.println();

  return pkt.footer == '\r';
}

bool serial_write(Packet & pkt)
{
  M5.Lcd.setCursor(0, 180);
  char send_packet[256] = {};
  if (!create_packet(&pkt, send_packet)) {return false;}
  Serial1.write(send_packet, pkt.data_len + 6); // Send packet to M5Core2

  // Debug print sent data to M5 LCD
  M5.Lcd.printf("=== SEND ===\n");
  M5.Lcd.printf("send_data len: %d\n", pkt.data_len);
  for (int i = 0; i < pkt.data_len + 6; ++i) {
    M5.Lcd.printf("%02X ", send_packet[i]);
  }
  M5.Lcd.println();
}

#endif // SERIAL_HANDLER_HPP
