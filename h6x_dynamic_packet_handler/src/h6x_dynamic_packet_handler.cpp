// Copyright 2025 HarvestX Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "h6x_dynamic_packet_handler/dynamic_packet_handler.hpp"

namespace h6x_dynamic_serial_interface
{

void concat_arrays(
  const uint8_t * a, const uint16_t len_a,
  const uint8_t * b, const uint16_t len_b,
  uint8_t * result, uint16_t * result_len)
{
  if (len_a + len_b > 245) {
    *result_len = 0;
    return;
  }

  memcpy(result, a, len_a);
  memcpy(result + len_a, b, len_b);
  *result_len = len_a + len_b;
}

bool packet_division(Packet * pkt, const char * data, const uint8_t recv_len)
{
  if (pkt == NULL || data == NULL) {
    return false;
  }

  pkt->mode = SERIAL_MODE_HOST;
  pkt->header = data[0];
  pkt->target_id = data[1];
  pkt->command = data[2];
  pkt->status = data[2];
  pkt->data_len = data[3];

  if (recv_len < pkt->data_len + 6 || pkt->data_len > 245) {
    return false;
  }

  for (int i = 0; i < pkt->data_len; i++) {
    pkt->data[i] = data[i + 4];
  }

  pkt->crc = data[pkt->data_len + 4];
  pkt->footer = data[pkt->data_len + 5];

  return true;
}


bool check_crc(const Packet * pkt)
{
  if (pkt == NULL) {
    return false;
  }

  uint8_t command_or_status = (pkt->mode == SERIAL_MODE_HOST) ? pkt->command : pkt->status;
  uint8_t crc_input[] = {pkt->header, pkt->target_id, command_or_status, pkt->data_len};
  uint8_t result[245] = {0};
  uint16_t result_len = 0;

  concat_arrays(crc_input, 4, pkt->data, pkt->data_len, result, &result_len);
  uint8_t crc_calc = crc8_calculate(result, result_len);

  return crc_calc == pkt->crc;
}


bool create_packet(const Packet * pkt, char * send_packet)
{
  if (pkt == NULL || send_packet == NULL) {
    return false;
  }

  send_packet[0] = pkt->header;
  send_packet[1] = pkt->target_id;
  send_packet[2] = (pkt->mode == SERIAL_MODE_HOST) ? pkt->command : pkt->status;
  send_packet[3] = pkt->data_len;
  memcpy(send_packet + 4, pkt->data, pkt->data_len);
  send_packet[pkt->data_len + 4] = crc8_calculate((const uint8_t *)send_packet, pkt->data_len + 4);
  send_packet[pkt->data_len + 5] = pkt->footer;

  return true;
}

} // namespace h6x_dynamic_serial_interface
