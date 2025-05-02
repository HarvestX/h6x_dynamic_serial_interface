#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include <M5Core2.h>
#include <string.h>
#include "protocol_definitions.hpp"
#include "crc8.hpp"
#include "command_handler.hpp"

/// @brief Read a serial packet and parse it into a ReceivedPacket structure
/// @param pkt Reference to a ReceivedPacket object to store parsed data
/// @return true if the packet is valid (footer matched), false otherwise
bool serial_read(ReceivedPacket & pkt)
{
  pkt.header = Serial.read();  // Read packet header
  if (pkt.header != '#') {
    return false;                        // Invalid header

  }
  // Read packet fields
  pkt.target_id = Serial.read();
  pkt.command = Serial.read();
  pkt.length = Serial.read();

  // Read payload data
  for (int i = 0; i < pkt.length; i++) {
    pkt.recv_data[i] = Serial.read();
  }

  pkt.crc_recv = Serial.read();  // Read received CRC
  pkt.footer = Serial.read();    // Read packet footer

  // Debug print received data to M5 LCD
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("=== RECV ===");
  M5.Lcd.printf("target_id: %02X\n", pkt.target_id);
  M5.Lcd.printf("command: %02X\n", pkt.command);
  M5.Lcd.print("recv_data: ");
  for (int i = 0; i < pkt.length; ++i) {
    M5.Lcd.printf("%02X ", pkt.recv_data[i]);
  }
  M5.Lcd.println();

  // Check footer (expected '\r')
  return pkt.footer == '\r';
}

/// @brief Handle a valid packet and send a response with CRC verification
/// @param pkt Reference to a ReceivedPacket object containing received data
void send_data(ReceivedPacket & pkt)
{
  // Prepare data for CRC check
  uint8_t crc_input[] = {pkt.target_id, pkt.command, pkt.length};
  uint8_t result[245] = {0};
  uint8_t result_len = 0;

  // Combine header and payload for CRC validation
  concat_arrays(crc_input, 3, pkt.recv_data, pkt.length, result, &result_len);
  uint8_t crc_calc = crc8_calculate(result, result_len);

  // Check CRC
  if (crc_calc == pkt.crc_recv) {
    // Execute command if CRC is valid
    command_handler(pkt);

    // Send response packet
    Serial.write('$');                        // Header
    Serial.write(OWN_ID);                    // Source ID
    Serial.write(pkt.status);                // Status
    Serial.write(pkt.send_data_len);         // Payload length

    for (int i = 0; i < pkt.send_data_len; i++) {
      Serial.write(pkt.send_data[i]);        // Payload
    }

    Serial.write(pkt.crc_send);              // CRC
    Serial.write('\r');                      // Footer

    // Debug print sent data to M5 LCD
    M5.Lcd.setCursor(0, 150);
    M5.Lcd.printf("=== SEND ===\n");
    M5.Lcd.printf("status: %02X\n", pkt.status);
    M5.Lcd.println();

  } else {
    // CRC mismatch warning
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.printf("CRC Mismatch!\n");
  }
}

#ifdef __cplusplus
}
#endif

#endif // PACKET_HANDLER_HPP
