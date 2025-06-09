#include <Arduino.h>
#include <M5Core2.h>
#include "packet_handler.h"
#include "command_handler.hpp"
#include "serial_handler.hpp"
#include "cpu_usage_handler.hpp"

ReceivedPacket pkt_send;
ReceivedPacket pkt_recv;

#define LENGTH 7

#define RX_PORTA 33
#define TX_PORTA 32

const char* options[] = {
  "00: Ping", "01: Change print color (red/green)", "02: Reboot device",
  "03: Request general status", "03: Request firmware version",
  "10: Request device tick", "12: Request firmware write date",
  "13: Request device vendor", "14: Request device name",
  "15: Request current state", "20: Display CPU usage",
  "250: Request output random number" // NEW
};

const uint8_t return_values[] = {
  0x00, 0x01, 0x02, 0x03, 0x03, 0x10, 0x12, 0x13, 0x14, 0x15, 0x20, 0xFA
};

const int num_options = sizeof(options) / sizeof(options[0]);
const int visible_rows = 6;
const int row_height = 40;

int selected_index = 0;
int menu_offset = 0;

void drawMenu(int highlightIndex = -1) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);

  for (int i = 0; i < visible_rows; i++) {
    int option_index = i + menu_offset;
    if (option_index >= num_options) break;

    if (option_index == highlightIndex) {
      M5.Lcd.setTextColor(BLACK, GREEN);
    } else {
      M5.Lcd.setTextColor(WHITE, BLACK);
    }

    M5.Lcd.fillRect(20, 30 + i * row_height, 280, row_height, (option_index == highlightIndex) ? GREEN : BLACK);
    M5.Lcd.setCursor(30, 30 + i * row_height + 8);
    M5.Lcd.println(options[option_index]);
  }
}

void sendSelectedCommand() {
  memset(&pkt_send, 0, sizeof(pkt_send));
  pkt_send.command = return_values[selected_index];
  if (pkt_send.command == 0x03) {
    pkt_send.send_data[0] = (selected_index == 3) ? 0x05 : 0x01;
    pkt_send.send_data_len = 1;
  }

  pkt_send.mode = SERIAL_MODE_PRIMARY;
  pkt_send.target_id = 0x01;
  pkt_send.device_id = PRIMARY_ID;
  if (!serial_write(pkt_send)) {
    M5.Lcd.fillRect(0, 260, 320, 40, BLACK);
    M5.Lcd.setCursor(20, 260);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.printf("Failed to send");
  } else {
    M5.Lcd.fillRect(0, 260, 320, 40, BLACK);
    M5.Lcd.setCursor(20, 260);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.printf("Sent: %s → 0x%02X", options[selected_index], return_values[selected_index]);
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.fillScreen(BLACK);
  Serial.begin(115200); // PC <--> M5Core2
  Serial1.begin(115200, SERIAL_8N1, RX_PORTA, TX_PORTA); // M5Core2 <--> M5Core2
  startCPUUsageMonitor();
  drawMenu(selected_index);
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    if (selected_index > 0) {
      selected_index--;
      if (selected_index < menu_offset) {
        menu_offset--;
      }
      drawMenu(selected_index);
    }
  }

  if (M5.BtnC.wasPressed()) {
    if (selected_index < num_options - 1) {
      selected_index++;
      if (selected_index >= menu_offset + visible_rows) {
        menu_offset++;
      }
      drawMenu(selected_index);
    }
  }

  if (M5.BtnB.wasPressed()) {
    sendSelectedCommand();
  }

  if (Serial1.available() >= LENGTH) {
    if (!serial_read(pkt_recv, SERIAL_MODE_PRIMARY, PRIMARY_ID)) {
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("Failed read\n");
      return;
    }
  }

  delay(20);
}
