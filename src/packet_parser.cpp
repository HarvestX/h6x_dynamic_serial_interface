#include "packet_parser.hpp"

bool read_basic_packet(ReceivedPacket& pkt) {
    pkt.device_id = Serial.read();
    pkt.target_id = Serial.read();
    pkt.command = Serial.read();
    pkt.crc_recv = Serial.read();
    pkt.footer = Serial.read();
    return pkt.footer == '\r';
}

bool read_send_data_packet(ReceivedPacket& pkt) {
    pkt.data = Serial.read();
    pkt.crc_recv = Serial.read();
    pkt.footer = Serial.read();
    return pkt.footer == '\r';
}

bool read_receive_data_packet(ReceivedPacket& pkt) {
    pkt.device_id = Serial.read();
    pkt.target_id = Serial.read();
    pkt.command = Serial.read();
    pkt.crc_recv = Serial.read();
    pkt.footer = Serial.read();
    return pkt.footer == '\r';
}

bool serial_read(ReceivedPacket& pkt) {
    pkt.header = Serial.read();
    if (pkt.header != '#') return false;

    switch (pkt.mode) {
        case MODE_BASIC:         return read_basic_packet(pkt);
        case MODE_SEND_DATA:     return read_send_data_packet(pkt);
        case MODE_RECEIVE_DATA:  return read_receive_data_packet(pkt);
        default:                 return false;
    }
}
