#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#include <Arduino.h>
#include "serial_comm.hpp"
#include "protocol_definitions.hpp"

struct ReceivedPacket;

void handle_basic_mode(const ReceivedPacket& pkt);
void handle_receive_data_mode(const ReceivedPacket& pkt);
void handle_send_data_mode(const ReceivedPacket& pkt);

#endif
