#ifndef PROTOCOL_DEFINITIONS_H
#define PROTOCOL_DEFINITIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "protocol_definitions_base.h"

#define OWN_ID 0x01
#define VERSION 0x01

// === CUSTOM COMMAND DEFINITIONS ===
#define CMD_REQUEST_IMU             0x20
#define CMD_REQUEST_CARRIPLATION_STATUS     0x21
#define CMD_REQUEST_CARRIPLATION_EXECUSION  0x96
#define CMD_REQUEST_OUTPUT_RANDOM_NUMBER    0xFA //NEW

// === CUSTOM ERROR CODES ===
#define NOW_CALIBRATING     0x05

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_DEFINITIONS_H
