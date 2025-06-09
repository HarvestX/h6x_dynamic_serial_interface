#include <Arduino.h>
#include <M5Core2.h>
#include "packet_handler.h"
#include "command_handler.hpp"
#include "serial_handler.hpp"

ReceivedPacket pkt;

#define LENGTH 7
#define ONW_ID 0x01

#define RX_PORTA 33
#define TX_PORTA 32

void setup()
{
  M5.begin();
  M5.IMU.Init();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);
  Serial.begin(115200); // PC <--> M5Core2
  Serial1.begin(115200, SERIAL_8N1, RX_PORTA, TX_PORTA); // M5Core2 <--> M5Core2
  unsigned long start_tick = millis();
}

void loop()
{
  memset(&pkt, 0, sizeof(pkt));
  if (Serial1.available() >= LENGTH) {
    if (!serial_read(pkt, SERIAL_MODE_SECONDARY, ONW_ID)) {
      M5.Lcd.setCursor(0, 100);
      M5.Lcd.printf("Failed read\n");
      return;
    }
    pkt.device_id = ONW_ID;  // Set device ID for response
    pkt.target_id = PRIMARY_ID;
    pkt.mode = SERIAL_MODE_SECONDARY;  // Set mode to secondary for response
    if (!serial_write(pkt)) {
      M5.Lcd.setCursor(0, 200);
      M5.Lcd.printf("Failed write\n");
      return;
    }
  }
  delay(10);   // ~100Hz loop
}
