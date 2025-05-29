#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "protocol_definitions_base.h"
#include "crc8.h"
#include "command_handler.hpp"


bool packet_division(ReceivedPacket & pkt, char * recv_packet, const int recv_len){
  // Parse the received packet into the ReceivedPacket structure
  pkt.header = recv_packet[0];  // Header
  pkt.target_id = recv_packet[1];  // Target ID
  pkt.command = recv_packet[2];  // Command
  pkt.length = recv_packet[3];  // Length of payload

  if (recv_len < pkt.length + 6 || pkt.length > 245) {
    return false; // Invalid length or insufficient data
  }
  // Read payload data
  for (int i = 0; i < pkt.length; i++) {
    pkt.recv_data[i] = recv_packet[i + 4];  // Payload data starts from index 4
  }

  pkt.crc_recv = recv_packet[pkt.length + 4];  // Received CRC
  pkt.footer = recv_packet[pkt.length + 5];    // Footer

  return true; // Successful parsing
}

bool check_crc(ReceivedPacket & pkt)
{
  // Prepare data for CRC check
  uint8_t crc_input[] = {pkt.target_id, pkt.command, pkt.length};
  uint8_t result[245] = {0};
  uint8_t result_len = 0;

  // Combine header and payload for CRC validation
  concat_arrays(crc_input, 3, pkt.recv_data, pkt.length, result, &result_len);
  uint8_t crc_calc = crc8_calculate(result, result_len);

  // Check CRC
  return crc_calc == pkt.crc_recv;
}

bool create_send_packet(ReceivedPacket & pkt, const uint8_t mode, const uint8_t id, char* send_packet)
{
  send_packet[0] = '$';                        // Header
  send_packet[1] = id;                    // Source ID
  if (mode == 1){
    send_packet[2] = pkt.command; // Command
  }
  send_packet[2 + mode] = pkt.status;                // Status
  send_packet[3 + mode] = pkt.send_data_len;         // Payload length

  memcpy(send_packet + mode + 4, pkt.send_data, pkt.send_data_len); // Payload
  send_packet[pkt.send_data_len + mode + 4] = pkt.crc_send; // CRC
  send_packet[pkt.send_data_len + mode + 5] = '\r'; // Footer

  return true; // Successful creation of send data
} 

#ifdef __cplusplus
}
#endif

#endif // PACKET_HANDLER_H
