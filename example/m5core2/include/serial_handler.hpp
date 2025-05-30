#ifndef SERIAL_HANDLER_HPP
#define SERIAL_HANDLER_HPP

#include <M5Core2.h>
#include <string.h>
#include "protocol_definitions.h"
#include "crc8.h"
#include "command_handler.hpp"
#include "packet_handler.h"
#include "cpu_usage_handler.hpp"


bool serial_read(ReceivedPacket & pkt)
{
  uint8_t header = Serial.read();  // Read packet header
  if (header != '#') {
    return false;                        // Invalid header
  }

  char recv_packet[256] = {header};
  int read_data_len = 1;
  int max_len = sizeof(recv_packet);

  while (read_data_len < max_len) {
    while (!Serial.available());
    int byte = Serial.read();
    if (byte == -1) continue;

    recv_packet[read_data_len++] = static_cast<char>(byte);

    if (byte == '\r') break;
  }
  packet_division(pkt, recv_packet, read_data_len);

  // Check CRC
  if (!check_crc(pkt)) {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("CRC error\n");
    return false;  // CRC check failed
  }

  // Debug print received data to M5 LCD
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("=== RECV ===\n");
  M5.Lcd.printf("header: %02X\n", pkt.header);
  M5.Lcd.printf("target_id: %02X\n", pkt.target_id);
  M5.Lcd.printf("command: %02X\n", pkt.command);
  M5.Lcd.printf("length: %02X\n", pkt.length);
  M5.Lcd.print("recv_data: ");
  for (int i = 0; i < pkt.length; ++i) {
    M5.Lcd.printf("%02X ", pkt.recv_data[i]);
  }
  M5.Lcd.println();
  M5.Lcd.printf("crc_recv: %02X\n", pkt.crc_recv);
  M5.Lcd.printf("footer: %02X\n", pkt.footer);
  M5.Lcd.println();

  // Check footer (expected '\r')
  return pkt.footer == '\r';
}

bool serial_write(ReceivedPacket & pkt, uint8_t mode)
{
  M5.Lcd.setCursor(0, 200);
  command_handler(pkt, mode);
  char send_packet[256] = {};
  if(!create_send_packet(pkt, send_packet, mode)) {return false;};
  Serial.write(send_packet, sizeof(send_packet));  // Send data to serial

  // Debug print sent data to M5 LCD
  M5.Lcd.printf("=== SEND ===\n");
  M5.Lcd.printf("send_data: ");
  for (int i = 0; i < pkt.send_data_len + 5; ++i) {
    M5.Lcd.printf("%02X ", send_packet[i]);
  }
  M5.Lcd.println();
}

#endif // SERIAL_HANDLER_HPP