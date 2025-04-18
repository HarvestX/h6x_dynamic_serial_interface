#include <Arduino.h>
#include <M5Core2.h>
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

    if (Serial.available() >= LENGTH) {
        if (!serial_read(pkt)) return;

        send_data(pkt);
    }

    delay(10); // ~100Hz loop
}
