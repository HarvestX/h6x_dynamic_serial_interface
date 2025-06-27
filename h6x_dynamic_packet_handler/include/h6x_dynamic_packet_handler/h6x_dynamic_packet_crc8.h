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

#ifndef H6X_DYNAMIC_PACKET_HANDLER__H6X_DYNAMIC_PACKET_CRC8_HPP
#define H6X_DYNAMIC_PACKET_HANDLER__H6X_DYNAMIC_PACKET_CRC8_HPP


#include <stdint.h>

uint8_t crc8_calculate(const uint8_t * input, const uint16_t len);

#endif // H6X_DYNAMIC_PACKET_HANDLER__H6X_DYNAMIC_PACKET_CRC8_HPP
