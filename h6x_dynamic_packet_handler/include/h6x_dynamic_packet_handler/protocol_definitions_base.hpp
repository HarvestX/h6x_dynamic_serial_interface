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

#ifndef H6X_DYNAMIC_PACKET_HANDLER__PROTOCOL_DEFINITIONS_BASE_HPP
#define H6X_DYNAMIC_PACKET_HANDLER__PROTOCOL_DEFINITIONS_BASE_HPP

#include <stdint.h>

#define DATA_LENGTH_MAX 245 // Maximum data_len of data in a packet


namespace h6x_dynamic_serial_interface
{

typedef enum SERIAL_MODE
{
  SERIAL_MODE_CLIENT = 0, // Secondary mode for communication
  SERIAL_MODE_HOST = 1,   // Primary mode for communication
  SERIAL_MODE_UNDEFINED = 2 // Undefined mode, used for error handling
} SERIAL_MODE;


typedef struct Packet
{
  uint8_t header;
  uint8_t target_id;
  SERIAL_MODE mode;
  uint8_t command;
  uint8_t status;
  uint8_t data_len;
  uint8_t data[DATA_LENGTH_MAX];
  uint8_t crc;
  uint8_t footer;
  uint32_t start_tick;
  uint32_t elapsed_tick;
} Packet;

Packet init_packet(const uint8_t target_id, const SERIAL_MODE mode);

// === COMMAND DEFINITIONS ===
typedef enum
{
  CMD_PING = 0x00,
  CMD_INTERNAL_LED_ON_OFF = 0x01,
  CMD_REBOOT_DEVICE = 0x02,
  CMD_REQUEST_GENERAL_STATUS = 0x03,
  CMD_REQUEST_FIRMWARE_VERSION = 0x03,
  CMD_REQUEST_DEVICE_TICK = 0x10,
  CMD_REQUEST_INTERNAL_ID = 0x11,
  CMD_REQUEST_FIRMWARE_WRITE_DATE = 0x12,
  CMD_REQUEST_DEVICE_VENDOR = 0x13,
  CMD_REQUEST_DEVICE_NAME = 0x14,
  CMD_REQUEST_CURRENT_STATE = 0x15
} PROTOCOL_COMMAND_BASE;

// === ERROR CODES ===
typedef enum
{
  ERR_SUCCESS = 0x00,
  ERR_FAILURE = 0x01,
  ERR_UNKNOWN_COMMAND = 0x02,
  ERR_CRC_ERROR = 0x03,
  ERR_TIMEOUT = 0x04,
  ERR_BUSY = 0x05,
  ERR_BUFFER_FULL = 0x06,
  ERR_INVALID_PACKET = 0x07,
  ERR_NOT_IMPLEMENTED = 0x08,
  ERR_OTHER = 0xFF
} PROTOCOL_ERROR_CODE;

#define PACKET_LENGTH_MIN 1

} // namespace h6x_dynamic_serial_interface


#endif // H6X_DYNAMIC_PACKET_HANDLER__PROTOCOL_DEFINITIONS_BASE_HPP
