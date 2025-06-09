#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "protocol_definitions_base.h"
#include "crc8.h"

void concat_arrays(
  const uint8_t * a, const uint8_t len_a,
  const uint8_t * b, const uint8_t len_b,
  uint8_t * result, uint8_t * result_len)
{
  if (len_a + len_b > 245) {
    *result_len = 0;    // Overflow check
    return;
  }

  memcpy(result, a, len_a);
  memcpy(result + len_a, b, len_b);
  *result_len = len_a + len_b;
}



bool packet_division(ReceivedPacket * pkt, char * recv_packet, int recv_len){
  if (pkt == NULL || recv_packet == NULL) {
    return false;
  }

  pkt->header = recv_packet[0];
  pkt->target_id = recv_packet[1];
  pkt->command = recv_packet[2];
  pkt->length = recv_packet[3];

  if (recv_len < pkt->length + 6 || pkt->length > 245) {
    return false;
  }

  for (int i = 0; i < pkt->length; i++) {
    pkt->recv_data[i] = recv_packet[i + 4];
  }

  pkt->crc_recv = recv_packet[pkt->length + 4];
  pkt->footer = recv_packet[pkt->length + 5];

  return true;
}

bool check_crc(ReceivedPacket * pkt)
{
  if (pkt == NULL) {
    return false;
  }

  uint8_t crc_input[] = {pkt->header, pkt->target_id, pkt->command, pkt->length};
  uint8_t result[245] = {0};
  uint8_t result_len = 0;

  concat_arrays(crc_input, 4, pkt->recv_data, pkt->length, result, &result_len);
  uint8_t crc_calc = crc8_calculate(result, result_len);

  return crc_calc == pkt->crc_recv;
}

bool create_callback_packet(ReceivedPacket * pkt, char * send_packet)
{
  if (pkt == NULL || send_packet == NULL) {
    return false;
  }

  send_packet[0] = (pkt->mode == SERIAL_MODE_PRIMARY) ? '#' : '$';
  send_packet[1] = (pkt->mode == SERIAL_MODE_PRIMARY) ? pkt->target_id : pkt->device_id;
  send_packet[2] = (pkt->mode == SERIAL_MODE_PRIMARY) ? pkt->command : pkt->status;
  send_packet[3] = pkt->send_data_len;
  memcpy(send_packet + 4, pkt->send_data, pkt->send_data_len);
  send_packet[pkt->send_data_len + 4] = crc8_calculate((const uint8_t *)send_packet, pkt->send_data_len + 4);
  send_packet[pkt->send_data_len + 5] = '\r';

  return true;
} 

#ifdef __cplusplus
}
#endif

#endif // PACKET_HANDLER_H
