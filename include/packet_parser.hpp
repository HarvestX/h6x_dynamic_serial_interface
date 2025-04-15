#ifndef PACKET_PARSER_HPP
#define PACKET_PARSER_HPP

#include <Arduino.h>

enum ProtocolMode {
    MODE_BASIC,
    MODE_RECEIVE_DATA,
    MODE_SEND_DATA
};

struct ReceivedPacket {
    ProtocolMode mode;
    uint8_t length;
    uint8_t header;
    uint8_t device_id;
    uint8_t target_id;
    uint8_t command;
    uint8_t crc_recv;
    uint8_t footer;
    uint8_t data; // Used only in SEND mode
};

bool serial_read(ReceivedPacket& pkt);

#endif
