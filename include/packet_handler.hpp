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
  uint8_t data; // Used only in SEND mode
};

void send_data(const ReceivedPacket& pkt);
bool serial_read(ReceivedPacket& pkt);
void handle_basic_mode(const ReceivedPacket& pkt);
void handle_receive_data_mode(const ReceivedPacket& pkt);
void handle_send_data_mode(const ReceivedPacket& pkt);

#endif
