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

  pkt->mode = (data[0] == HEADER_HOST) ? SERIAL_MODE_HOST : SERIAL_MODE_CLIENT;
  pkt->client_id = data[1];
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
  if (data[pkt->data_len + 5] != '\r') {
    return false;
  }
  return true;
}


bool check_crc(const Packet * pkt)
{
  if (pkt == NULL) {
    return false;
  }

  uint8_t command_or_status = (pkt->mode == SERIAL_MODE_HOST) ? pkt->command : pkt->status;
  uint8_t crc_input[] =
  {((pkt->mode == SERIAL_MODE_HOST) ? HEADER_HOST : HEADER_CLIENT), pkt->client_id,
    command_or_status, pkt->data_len};
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

  send_packet[0] = (pkt->mode == SERIAL_MODE_HOST) ? HEADER_HOST : HEADER_CLIENT;
  send_packet[1] = pkt->client_id;
  send_packet[2] = (pkt->mode == SERIAL_MODE_HOST) ? pkt->command : pkt->status;
  send_packet[3] = pkt->data_len;
  memcpy(send_packet + 4, pkt->data, pkt->data_len);
  send_packet[pkt->data_len + 4] = crc8_calculate((const uint8_t *)send_packet, pkt->data_len + 4);
  send_packet[pkt->data_len + 5] = '\r';

  return true;
}

Packet get_received_packet(const char * input, const int32_t input_len, const uint8_t client_id)
{
  Packet r_pkt = init_packet();

  if (input_len < ADDITIONAL_PACKET_LENGTH + 1 || input == NULL) {
    r_pkt.is_valid = false;
    return r_pkt;
  }

  if (input[0] != HEADER_HOST && input[0] != HEADER_CLIENT) {
    r_pkt.is_valid = false;
    return r_pkt;
  }

  r_pkt.mode = (input[0] == HEADER_HOST) ? SERIAL_MODE_HOST : SERIAL_MODE_CLIENT;

  if (!packet_division(&r_pkt, input, input_len)) {
    r_pkt.is_valid = false;
    return r_pkt;
  }

  if (r_pkt.client_id != client_id) {
    r_pkt.is_valid = false;
    return r_pkt;
  }

  if (!check_crc(&r_pkt)) {
    r_pkt.is_valid = false;
    return r_pkt;
  }
  r_pkt.is_valid = true;
  return r_pkt;
}


uint8_t get_serial_data(char * input_buf, const int max_len, serial_getchar_fn_t getchar_fn)
{
  uint8_t idx = 0;
  bool start_bit = false;
  uint8_t expected_len = 0;

  while (idx < max_len - 1) {
    char c = getchar_fn(1000 * 100);

    if (!start_bit && c == '#') {
      start_bit = true;
      idx = 1;
    } else if (start_bit && c >= 0) {
      input_buf[idx++] = c;
      if (idx == 4) {
        expected_len = (uint8_t)input_buf[idx - 1];
      } else if (expected_len && idx == expected_len + ADDITIONAL_PACKET_LENGTH) {
        break;
      }
    }
  }
  return idx;
}

} // namespace h6x_dynamic_serial_interface
