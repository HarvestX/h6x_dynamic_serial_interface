#ifndef PROTOCOL_DEFINITIONS_BASE_H
#define PROTOCOL_DEFINITIONS_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define DATA_LENGTH_MAX 245 // Maximum data_len of data in a packet

typedef enum SERIAL_MODE {
  SERIAL_MODE_SECONDARY = 0, // Secondary mode for communication
  SERIAL_MODE_PRIMARY = 1,   // Primary mode for communication
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

Packet init_packet(const uint8_t target_id, const SERIAL_MODE mode)
{
  Packet pkt;
  pkt.header = (mode == SERIAL_MODE_PRIMARY) ? (uint8_t)'#' : (uint8_t)'$';
  pkt.mode = mode;
  pkt.target_id = target_id;
  pkt.footer = '\r';
  return pkt;
}

// === COMMAND DEFINITIONS ===
#define CMD_PING                    0x00
#define CMD_INTERNAL_LED_ON_OFF     0x01
#define CMD_REBOOT_DEVICE           0x02
#define CMD_REQUEST_GENERAL_STATUS  0x03
#define CMD_REQUEST_FIRMWARE_VERSION 0x03
#define CMD_REQUEST_DEVICE_TICK     0x10
#define CMD_REQUEST_INTERNAL_ID     0x11
#define CMD_REQUEST_FIRMWARE_WRITE_DATE 0x12
#define CMD_REQUEST_DEVICE_VENDOR   0x13
#define CMD_REQUEST_DEVICE_NAME     0x14
#define CMD_REQUEST_CURRENT_STATE   0x15

// === ERROR CODES ===
#define ERR_SUCCESS         0x00
#define ERR_FAILURE         0x01
#define ERR_UNKNOWN_COMMAND 0x02
#define ERR_CRC_ERROR       0x03
#define ERR_TIMEOUT         0x04
#define ERR_BUSY            0x05
#define ERR_BUFFER_FULL     0x06
#define ERR_OTHER           0xFF

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_DEFINITIONS_BASE_H
