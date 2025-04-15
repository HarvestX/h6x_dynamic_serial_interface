#include <Arduino.h>
#include <M5Core2.h>
#include "serial_comm.hpp"
#include "protocol_definitions.hpp"

#define MODE 0

enum ProtocolMode {
    MODE_BASIC,         // Basic command
    MODE_RECEIVE_DATA,  // Data request command
    MODE_SEND_DATA      // Data transmission (PC → M5)
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
    uint8_t data;  // Used only in SEND mode
};

void serial_read(ReceivedPacket& pkt) {
    pkt.header = Serial.read();
    if (pkt.header != '#') return;

    if (pkt.mode == MODE_BASIC) {
        // Format 1: Basic / Receive mode (# + ID + ID + CMD + CRC + \r)
        pkt.device_id = Serial.read();
        pkt.target_id = Serial.read();
        pkt.command = Serial.read();
        pkt.crc_recv = Serial.read();
        pkt.footer = Serial.read();
        if (pkt.footer != '\r') return;

    } else if (pkt.mode == MODE_SEND_DATA) {
        // Format 2: Data send mode (# + DATA + CRC + \r)
        M5.Lcd.setCursor(0, 70);
        M5.Lcd.printf("MODE_SEND_DATA\n");
        pkt.data = Serial.read();
        pkt.crc_recv = Serial.read();
        pkt.footer = Serial.read();
        if (pkt.footer != '\r') return;

    } else if (pkt.mode == MODE_RECEIVE_DATA) {
        // Format 3: Data request command (# + ID + ID + CMD + CRC + \r)
        pkt.device_id = Serial.read();
        pkt.target_id = Serial.read();
        pkt.command = Serial.read();
        pkt.crc_recv = Serial.read();
        pkt.footer = Serial.read();
        if (pkt.footer != '\r') return;
    }
}

void setup() {
    M5.begin();
    M5.IMU.Init();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setTextSize(2);
    Serial.begin(115200);
}

void loop() {
    ReceivedPacket pkt;

    if (MODE == 0) {
        pkt.mode = MODE_BASIC;
        pkt.length = 6;
    } else if (MODE == 1) {
        pkt.mode = MODE_RECEIVE_DATA;
        pkt.length = 6;
    } else if (MODE == 2) {
        pkt.mode = MODE_SEND_DATA;
        pkt.length = 4;
    }

    if (Serial.available() >= pkt.length) {
        serial_read(pkt);
        
        if (pkt.mode == MODE_BASIC) {
            // Format 1: Basic / Receive mode

            uint8_t crc_input[] = {
                pkt.device_id,
                pkt.target_id,
                pkt.command
            };
            uint8_t crc_calc = crc8_calculate(crc_input, 3);
            M5.Lcd.setCursor(0, 50);
            M5.Lcd.printf("crc_calc: %d\n", crc_calc);
    
            if (crc_calc == pkt.crc_recv) {
                // Send response (own_id, status, crc8, \r)
                uint8_t own_id = pkt.header;
                uint8_t status = 0x55;
                uint8_t response[2] = {own_id, status};
                uint8_t crc = crc8_calculate(response, 2);
    
                Serial.write(own_id);
                Serial.write(status);
                Serial.write(crc);
                Serial.write('\r');
            } else {
                M5.Lcd.setCursor(0, 220);
                M5.Lcd.printf("CRC Mismatch!\n");
            }
            
        } else if(pkt.mode == MODE_RECEIVE_DATA) {
            // Format 3: Data request command

            uint8_t crc_input[] = {
                pkt.device_id,
                pkt.target_id,
                pkt.command
            };
            uint8_t crc_calc = crc8_calculate(crc_input, 3);
            if (crc_calc == pkt.crc_recv) {
                uint8_t own_id = pkt.header;
                uint8_t data = 0xBB;
                uint8_t response[2] = {own_id, data};
                uint8_t crc = crc8_calculate(response, 2);

                Serial.write(own_id);
                Serial.write(data);
                Serial.write(crc);
                Serial.write('\r');
            } else {
                M5.Lcd.setCursor(0, 220);
                M5.Lcd.printf("CRC Mismatch!\n");
            }
        } else if (pkt.mode == MODE_SEND_DATA) {
            // Format 2: Data send mode

            M5.Lcd.setCursor(0, 20);
            M5.Lcd.printf("data: %d, crc: %d\n", pkt.data, pkt.crc_recv);

            if (pkt.footer != '\r') return;

            uint8_t crc_input[] = {
                pkt.data
            };
            uint8_t crc_calc = crc8_calculate(crc_input, 1);
            M5.Lcd.setCursor(0, 50);
            M5.Lcd.printf("crc_calc: %d\n", crc_calc);

            if (crc_calc == pkt.crc_recv) {
                uint8_t own_id = pkt.header;
                uint8_t status = 0x55;
                uint8_t response[2] = {own_id, status};
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
    }
    delay(10);  // ~100Hz loop
}
