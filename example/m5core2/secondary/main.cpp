#include <Arduino.h>
#include <M5Core2.h>
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_handler.h"
#include "command_handler.hpp"
#include "serial_handler.hpp"

Packet pkt_recv;
Packet pkt_send;

#define DATA_LEN 7
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
  memset(&pkt_recv, 0, sizeof(pkt_recv));
  if (Serial1.available() >= DATA_LEN) {
    if (!serial_read(pkt_recv, SERIAL_MODE_CLIENT, ONW_ID)) {
      M5.Lcd.setCursor(0, 100);
      M5.Lcd.printf("Failed read\n");
      return;
    }
    pkt_send = init_packet();
    command_handler(pkt_recv.command, pkt_send);
    pkt_send.start_tick = pkt_recv.start_tick;
    pkt_send.elapsed_tick = millis() - pkt_recv.start_tick;
    if (!serial_write(pkt_send)) {
      M5.Lcd.setCursor(0, 200);
      M5.Lcd.printf("Failed write\n");
      return;
    }
  }
  delay(10);   // ~100Hz loop
}
