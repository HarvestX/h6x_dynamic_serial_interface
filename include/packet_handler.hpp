#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#include <Arduino.h>
#include "protocol_definitions.hpp"

enum ProtocolMode {
  MODE_BASIC,
  MODE_RECEIVE_DATA,
  MODE_SEND_DATA
};

struct ReceivedPacket {
  ProtocolMode mode;
  uint8_t length;
  uint8_t header;
  uint8_t device_id;
  uint8_t target_id;
  uint8_t command;
  uint8_t status;
  uint8_t crc_recv;
  uint8_t footer;
  uint8_t data[245];
};

void send_data(const ReceivedPacket& pkt);
bool serial_read(ReceivedPacket& pkt);
void command_handler(ReceivedPacket& pkt);
void ping_processor(ReceivedPacket& pkt);
void led_processor(ReceivedPacket& pkt);
void reboot_processor(ReceivedPacket& pkt);
void request_processor(ReceivedPacket& pkt);
void request_firmware_version(ReceivedPacket& pkt);
void request_device_tick(ReceivedPacket& pkt);
void request_internal_id(ReceivedPacket& pkt);
void request_firmware_write_date(ReceivedPacket& pkt);
void request_device_vendor(ReceivedPacket& pkt);
void request_device_name(ReceivedPacket& pkt);
void request_current_state(ReceivedPacket& pkt);
void request_general_status(ReceivedPacket& pkt);

#endif
