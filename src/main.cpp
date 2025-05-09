#include <Arduino.h>
#include <M5Core2.h>
#include "packet_handler.h"
#include "command_handler.hpp"
#include "serial_handler.hpp"

ReceivedPacket pkt;

#define LENGTH 7

void setup()
{
  M5.begin();
  M5.IMU.Init();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);
  Serial.begin(115200);
  unsigned long start_tick = millis();
}

void loop()
{
  memset(&pkt, 0, sizeof(pkt));
  if (Serial.available() >= LENGTH) {
    if (!serial_read(pkt)) {
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("Failed read\n");
      return;
    }
    if (!serial_write(pkt)) {
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("Failed write\n");
      return;
    }
  }

  delay(10);   // ~100Hz loop
}
