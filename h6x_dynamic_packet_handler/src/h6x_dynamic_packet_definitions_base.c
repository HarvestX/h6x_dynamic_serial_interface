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

#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_definitions_base.h"


Packet init_packet()
{
  Packet pkt;
  pkt.mode = SERIAL_MODE_UNDEFINED;
  pkt.status = ERR_SUCCESS;
  pkt.data_len = DATA_LENGTH_MIN;
  pkt.is_valid = true;
  return pkt;
}
