#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#pragma once
#include <M5Core2.h>
#include <string.h>
#include "protocol_definitions.hpp"
#include "crc8.hpp"

void concat_arrays(const uint8_t* a, uint8_t len_a,
  const uint8_t* b, uint8_t len_b,
  uint8_t* result, uint8_t* result_len)
{
  if (len_a + len_b > 245) {
      *result_len = 0;  // 結合できない
      return;
  }

  memcpy(result, a, len_a);          // 先頭にaをコピー
  memcpy(result + len_a, b, len_b);  // aの後ろにbをコピー

  *result_len = len_a + len_b;
}

void ping_processor(ReceivedPacket& pkt) {
  if (pkt.command == CMD_PING) {
      uint8_t crc_data_len = 0;
      uint8_t send_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
      memcpy(pkt.send_data, send_data, sizeof(send_data));
      pkt.send_data_len = 4;
      pkt.status = 0x01;
      uint8_t response_len = 3;
      uint8_t response[response_len] = {OWN_ID, pkt.status, pkt.send_data_len};
      uint8_t crc_data[pkt.send_data_len + response_len];
      std::copy(response, response + response_len, crc_data);
      std::copy(pkt.send_data, pkt.send_data + pkt.send_data_len, crc_data + response_len);
      crc_data_len = response_len + pkt.send_data_len;

      M5.Lcd.printf("crc_data: ");
      for (int i = 0; i < crc_data_len; ++i) {
          M5.Lcd.printf("%02X ", crc_data[i]);
      }
      M5.Lcd.println();
      pkt.crc_send = crc8_calculate(crc_data, crc_data_len);
  } 
}

void led_processor(ReceivedPacket& pkt) {
  if (pkt.command == CMD_INTERNAL_LED_ON_OFF) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}

void reboot_processor(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REBOOT_DEVICE) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_processor(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_GENERAL_STATUS) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_firmware_version(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_FIRMWARE_VERSION) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_device_tick(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_DEVICE_TICK) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_internal_id(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_INTERNAL_ID) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_firmware_write_date(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_FIRMWARE_WRITE_DATE) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_device_vendor(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_DEVICE_VENDOR) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_device_name(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_DEVICE_NAME) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_current_state(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_CURRENT_STATE) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}
void request_general_status(ReceivedPacket& pkt) {
  if (pkt.command == CMD_REQUEST_GENERAL_STATUS) {
      pkt.status = ERR_SUCCESS;
      //pkt.data = 0x00;
  }
}

void command_handler(ReceivedPacket& pkt) {
  switch (pkt.command) {
      case CMD_PING:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("PING command\n");
          ping_processor(pkt);
          break;
      case CMD_INTERNAL_LED_ON_OFF:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("LED command\n");
          break;
      case CMD_REBOOT_DEVICE:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("REBOOT command\n");
          reboot_processor(pkt);
          break;
      case CMD_REQUEST_GENERAL_STATUS:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("STATUS command\n");
          M5.Lcd.printf("VERSION command\n");
          request_processor(pkt);
          request_firmware_version(pkt);
          break;
      case CMD_REQUEST_DEVICE_TICK:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("TICK command\n");
          request_device_tick(pkt);
          break;
      case CMD_REQUEST_INTERNAL_ID:  
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("ID command\n");
          request_internal_id(pkt);
          break;
      case CMD_REQUEST_FIRMWARE_WRITE_DATE:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("DATE command\n");
          request_firmware_write_date(pkt);
          break;
      case CMD_REQUEST_DEVICE_VENDOR:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("VENDOR command\n");
          request_device_vendor(pkt);
          break;
      case CMD_REQUEST_DEVICE_NAME:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("NAME command\n");
          request_device_name(pkt);
          break;
      case CMD_REQUEST_CURRENT_STATE:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("STATE command\n");
          request_current_state(pkt);
          break;
      default:
          M5.Lcd.setCursor(0, 150);
          M5.Lcd.printf("Unknown command\n");
  }
}

#endif