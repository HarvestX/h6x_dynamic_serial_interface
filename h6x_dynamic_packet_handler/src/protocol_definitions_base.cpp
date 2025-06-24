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

#include "h6x_dynamic_packet_handler/protocol_definitions_base.hpp"


namespace h6x_dynamic_serial_interface
{
Packet init_packet(const uint8_t target_id, const SERIAL_MODE mode)
{
  Packet pkt;
  pkt.header = (mode == SERIAL_MODE_HOST) ? (uint8_t)'#' : (uint8_t)'$';
  pkt.mode = mode;
  pkt.target_id = target_id;
  pkt.footer = '\r';
  return pkt;
}
} // namespace h6x_dynamic_serial_interface
