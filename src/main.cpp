#include <Arduino.h>
#include <M5Core2.h>
#include "packet_parser.hpp"
#include "packet_handler.hpp"

#define MODE 0

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
        if (!serial_read(pkt)) return;

        switch (pkt.mode) {
            case MODE_BASIC:         handle_basic_mode(pkt); break;
            case MODE_RECEIVE_DATA:  handle_receive_data_mode(pkt); break;
            case MODE_SEND_DATA:     handle_send_data_mode(pkt); break;
        }
    }

    delay(10); // ~100Hz loop
}
