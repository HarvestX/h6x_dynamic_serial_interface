#include <M5Core2.h>
#include "crc8.hpp"
#include "packet_parser.hpp"
#include "serial_comm.hpp"
#

void handle_basic_mode(const ReceivedPacket& pkt) {
    uint8_t crc_input[] = { pkt.device_id, pkt.target_id, pkt.command };
    uint8_t crc_calc = crc8_calculate(crc_input, 3);

    if (crc_calc == pkt.crc_recv) {
        uint8_t own_id = pkt.header;
        uint8_t status = 0x55;
        uint8_t response[2] = { own_id, status };
        uint8_t crc = crc8_calculate(response, 2);

        Serial.write(own_id);
        Serial.write(status);
        Serial.write(crc);
        Serial.write('\r');
    } else {
        M5.Lcd.setCursor(0, 220);
        M5.Lcd.printf("CRC Mismatch!\n");
    }
}

void handle_receive_data_mode(const ReceivedPacket& pkt) {
    uint8_t crc_input[] = { pkt.device_id, pkt.target_id, pkt.command };
    uint8_t crc_calc = crc8_calculate(crc_input, 3);

    if (crc_calc == pkt.crc_recv) {
        uint8_t own_id = pkt.header;
        uint8_t data = 0xBB;
        uint8_t response[2] = { own_id, data };
        uint8_t crc = crc8_calculate(response, 2);

        Serial.write(own_id);
        Serial.write(data);
        Serial.write(crc);
        Serial.write('\r');
    } else {
        M5.Lcd.setCursor(0, 220);
        M5.Lcd.printf("CRC Mismatch!\n");
    }
}

void handle_send_data_mode(const ReceivedPacket& pkt) {
    uint8_t crc_input[] = { pkt.data };
    uint8_t crc_calc = crc8_calculate(crc_input, 1);

    if (crc_calc == pkt.crc_recv) {
        uint8_t own_id = pkt.header;
        uint8_t status = 0x55;
        uint8_t response[2] = { own_id, status };
        uint8_t crc = crc8_calculate(response, 2);

        Serial.write(own_id);
        Serial.write(status);
        Serial.write(crc);
        Serial.write('\r');
    } else {
        M5.Lcd.setCursor(0, 220);
        M5.Lcd.printf("CRC Mismatch!\n");
    }
}
