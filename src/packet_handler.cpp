#include <M5Core2.h>
#include "crc8.hpp"
#include "protocol_definitions.hpp"
#include "packet_handler.hpp"

bool serial_read(ReceivedPacket& pkt) {
    pkt.header = Serial.read();
    if (pkt.header != '#') return false;

    pkt.target_id = Serial.read();
    pkt.command = Serial.read();
    pkt.length = Serial.read();
    pkt.data = Serial.read();
    pkt.crc_recv = Serial.read();
    pkt.footer = Serial.read();

    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("target_id: %02X\n", pkt.target_id);
    M5.Lcd.printf("command: %02X\n", pkt.command);
    M5.Lcd.printf("length: %02X\n", pkt.length);
    M5.Lcd.printf("data: %02X\n", pkt.data);
    M5.Lcd.printf("crc_recv: %02X\n", pkt.crc_recv);
    M5.Lcd.printf("footer: %02X\n", pkt.footer);

    return pkt.footer == '\r';
}

void send_data(const ReceivedPacket& pkt){
    uint8_t status = 0x05;
    uint8_t crc_input[] = {pkt.target_id, pkt.command, pkt.length, pkt.data};
    uint8_t crc_calc = crc8_calculate(crc_input, 4);

    if (crc_calc == pkt.crc_recv) {;
        uint8_t response[4] = { OWN_ID, status, pkt.length, pkt.data };
        uint8_t crc = crc8_calculate(response, 4);

        M5.Lcd.setCursor(0, 220);
        M5.Lcd.printf("CRC : %02X\n", crc);

        Serial.write('$');
        Serial.write(OWN_ID);
        Serial.write(status);
        Serial.write(pkt.length);
        Serial.write(pkt.data);
        Serial.write(crc);
        Serial.write('\r');
    } else {
        M5.Lcd.setCursor(0, 220);
        M5.Lcd.printf("CRC Mismatch!\n");
    }
}
