// protocol_definitions.hpp

#ifndef PROTOCOL_DEFINITIONS_HPP
#define PROTOCOL_DEFINITIONS_HPP

// === COMMAND DEFINITIONS ===
#define DEVICE_ID 0x01
#define CMD_PING                     0x00
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

#endif // PROTOCOL_DEFINITIONS_HPP
