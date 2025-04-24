#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#pragma once
#include <M5Core2.h>
#include <string.h>
#include "protocol_definitions.hpp"
#include "crc8.hpp"
#include "command_handler.hpp"

bool serial_read(ReceivedPacket& pkt) {
  pkt.header = Serial.read();
  if (pkt.header != '#') return false;

  pkt.target_id = Serial.read();
  pkt.command = Serial.read();
  pkt.length = Serial.read();

  for(int i = 0; i < pkt.length; i++) {
      pkt.recv_data[i] = Serial.read();
  }

  pkt.crc_recv = Serial.read();
  pkt.footer = Serial.read();

  // M5.Lcd.setCursor(0, 20);
  // M5.Lcd.printf("target_id: %02X\n", pkt.target_id);
  // M5.Lcd.printf("command: %02X\n", pkt.command);
  // M5.Lcd.printf("length: %02X\n", pkt.length);
  
  // M5.Lcd.print("recv_data: ");
  // for (int i = 0; i < pkt.length; ++i) {
  //     M5.Lcd.printf("%02X ", pkt.recv_data[i]);
  // }
  // M5.Lcd.println();

  // M5.Lcd.printf("crc_recv: %02X\n", pkt.crc_recv);
  // M5.Lcd.printf("footer: %02X\n", pkt.footer);

  return pkt.footer == '\r';
}

void send_data(ReceivedPacket& pkt){
  uint8_t crc_input[] = {pkt.target_id, pkt.command, pkt.length};
  uint8_t result[245] = {0};
  uint8_t result_len = 0;
  concat_arrays(crc_input, 3, pkt.recv_data, pkt.length, result, &result_len);

  uint8_t crc_calc = crc8_calculate(result, result_len);

  if (crc_calc == pkt.crc_recv) {;
      command_handler(pkt);

      M5.Lcd.setCursor(0, 220);
      M5.Lcd.printf("CRC : %02X\n", pkt.crc_send);

      Serial.write('$');
      Serial.write(OWN_ID);
      Serial.write(pkt.status);
      Serial.write(pkt.send_data_len);

      for(int i = 0; i < pkt.send_data_len; i++) {
      
          Serial.write(pkt.send_data[i]);
      }
      Serial.write(pkt.crc_send);
      Serial.write('\r');

      M5.Lcd.setCursor(0, 170);
      M5.Lcd.printf("send_data: ");
      for (int i = 0; i < pkt.send_data_len; ++i) {
          M5.Lcd.printf("%02X ", pkt.send_data[i]);
      }

  } else {
      M5.Lcd.setCursor(0, 220);
      M5.Lcd.printf("CRC Mismatch!\n");
  }
}

#endif
