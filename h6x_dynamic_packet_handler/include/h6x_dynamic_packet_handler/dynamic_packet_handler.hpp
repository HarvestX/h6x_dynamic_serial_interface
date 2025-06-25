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

#ifndef H6X_DYNAMIC_PACKET_HANDLER__DYNAMIC_PACKET_HANDLER_HPP
#define H6X_DYNAMIC_PACKET_HANDLER__DYNAMIC_PACKET_HANDLER_HPP

#include <stdint.h>
#include <stdbool.h>


#include <string.h>
#include "h6x_dynamic_packet_handler/protocol_definitions_base.hpp"
#include "h6x_dynamic_packet_handler/crc8.hpp"


namespace h6x_dynamic_serial_interface
{
typedef char (* serial_getchar_fn_t)(uint32_t timeout_us);

uint8_t get_serial_data(char * input_buf, const int max_len, serial_getchar_fn_t getchar_fn);

void concat_arrays(
  const uint8_t * a, const uint16_t len_a,
  const uint8_t * b, const uint16_t len_b,
  uint8_t * result, uint16_t * result_len);


bool packet_division(Packet * pkt, const char * data, const uint8_t recv_len);

bool check_crc(const Packet * pkt);

bool create_packet(const Packet * pkt, char * send_packet);

Packet get_received_packet(const char * input, const int32_t input_len, const uint8_t client_id);

int32_t process_msg(const char * input, int32_t input_len, char * output, int32_t output_max_len);

} // namespace h6x_dynamic_serial_interface


#endif // H6X_DYNAMIC_PACKET_HANDLER__DYNAMIC_PACKET_HANDLER_HPP
