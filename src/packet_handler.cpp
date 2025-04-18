#include <M5Core2.h>
#include <string.h>
#include "crc8.hpp"
#include "protocol_definitions.hpp"
#include "packet_handler.hpp"

void concat_arrays(const uint8_t* a, uint8_t len_a,
    const uint8_t* b, uint8_t len_b,
    uint8_t* result, uint8_t* result_len)
{
    if (len_a + len_b > 245) {
        *result_len = 0;  // 結合できない
        return;
    }

    memcpy(result, a, len_a);          // 先頭にaをコピー
    memcpy(result + len_a, b, len_b);  // aの後ろにbをコピー

    *result_len = len_a + len_b;
}

bool serial_read(ReceivedPacket& pkt) {
    pkt.header = Serial.read();
    if (pkt.header != '#') return false;

    pkt.target_id = Serial.read();
    pkt.command = Serial.read();
    pkt.length = Serial.read();

    for(int i = 0; i < pkt.length; i++) {
        pkt.data[i] = Serial.read();
    }

    pkt.crc_recv = Serial.read();
    pkt.footer = Serial.read();

    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("target_id: %02X\n", pkt.target_id);
    M5.Lcd.printf("command: %02X\n", pkt.command);
    M5.Lcd.printf("length: %02X\n", pkt.length);
    
    M5.Lcd.print("data: ");
    for (int i = 0; i < pkt.length; ++i) {
        M5.Lcd.printf("%02X ", pkt.data[i]);
    }
    M5.Lcd.println();

    M5.Lcd.printf("crc_recv: %02X\n", pkt.crc_recv);
    M5.Lcd.printf("footer: %02X\n", pkt.footer);

    return pkt.footer == '\r';
}

void send_data(const ReceivedPacket& pkt){
    uint8_t status = 0x05;
    uint8_t crc_input[] = {pkt.target_id, pkt.command, pkt.length};
    uint8_t result[245] = {0};
    uint8_t result_len = 0;
    concat_arrays(crc_input, 3, pkt.data, pkt.length, result, &result_len);

    uint8_t crc_calc = crc8_calculate(result, result_len);

    if (crc_calc == pkt.crc_recv) {;
        uint8_t send_data[245] = {};
        uint8_t send_data_len = 0;

        uint8_t data[245] = {0xAA, 0xBB, 0xCC, 0xDD}; 
        uint8_t data_len = 4;
        uint8_t response[3] = { OWN_ID, status, data_len};
        concat_arrays(response, 3, data, data_len, send_data, &send_data_len);
        
        uint8_t crc = crc8_calculate(send_data, send_data_len);

        M5.Lcd.setCursor(0, 220);
        M5.Lcd.printf("CRC : %02X\n", crc);

        Serial.write('$');
        Serial.write(OWN_ID);
        Serial.write(status);
        Serial.write(pkt.length);

        for(int i = 0; i < pkt.length; i++) {
            Serial.write(data[i]);
        }

        Serial.write(crc);
        Serial.write('\r');
    } else {
        M5.Lcd.setCursor(0, 220);
        M5.Lcd.printf("CRC Mismatch!\n");
    }
}

void command_handler(ReceivedPacket& pkt) {
    switch (pkt.command) {
        case CMD_PING:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("PING command\n");
            ping_processor(pkt);
            break;
        case CMD_INTERNAL_LED_ON_OFF:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("LED command\n");
            break;
        case CMD_REBOOT_DEVICE:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("REBOOT command\n");
            reboot_processor(pkt);
            break;
        case CMD_REQUEST_GENERAL_STATUS:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("STATUS command\n");
            M5.Lcd.printf("VERSION command\n");
            request_processor(pkt);
            request_firmware_version(pkt);
            break;
        case CMD_REQUEST_DEVICE_TICK:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("TICK command\n");
            request_device_tick(pkt);
            break;
        case CMD_REQUEST_INTERNAL_ID:  
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("ID command\n");
            request_internal_id(pkt);
            break;
        case CMD_REQUEST_FIRMWARE_WRITE_DATE:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("DATE command\n");
            request_firmware_write_date(pkt);
            break;
        case CMD_REQUEST_DEVICE_VENDOR:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("VENDOR command\n");
            request_device_vendor(pkt);
            break;
        case CMD_REQUEST_DEVICE_NAME:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("NAME command\n");
            request_device_name(pkt);
            break;
        case CMD_REQUEST_CURRENT_STATE:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("STATE command\n");
            request_current_state(pkt);
            break;
        default:
            M5.Lcd.setCursor(0, 150);
            M5.Lcd.printf("Unknown command\n");
    }
}

void ping_processor(ReceivedPacket& pkt) {
    if (pkt.command == CMD_PING) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}

void led_processor(ReceivedPacket& pkt) {
    if (pkt.command == CMD_INTERNAL_LED_ON_OFF) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}

void reboot_processor(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REBOOT_DEVICE) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_processor(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_GENERAL_STATUS) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_firmware_version(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_FIRMWARE_VERSION) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_device_tick(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_DEVICE_TICK) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_internal_id(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_INTERNAL_ID) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_firmware_write_date(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_FIRMWARE_WRITE_DATE) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_device_vendor(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_DEVICE_VENDOR) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_device_name(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_DEVICE_NAME) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_current_state(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_CURRENT_STATE) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
void request_general_status(ReceivedPacket& pkt) {
    if (pkt.command == CMD_REQUEST_GENERAL_STATUS) {
        pkt.status = ERR_SUCCESS;
        //pkt.data = 0x00;
    }
}
