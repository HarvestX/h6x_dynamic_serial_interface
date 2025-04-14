#include <Arduino.h>
#include <M5Core2.h>
#include "imu_filter.hpp"
#include "data_logger.hpp"
#include "serial_comm.hpp"
#include "protocol_definitions.hpp"

#define CSV_FILENAME "/data/imu_data.csv"
#define BUFFER_SIZE 50

std::vector<String> dataBuffer;
float offsetX = 0, offsetY = 0, offsetZ = 0;

void setup() {
    M5.begin();
    M5.IMU.Init();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setTextSize(2);
    Serial.begin(115200);

    if (!initFileSystem(CSV_FILENAME)) {
        while (1);
    }

    calibrateIMU(offsetX, offsetY, offsetZ);
}

void loop() {
    float accX, accY, accZ, gyroX, gyroY, gyroZ, pitch, roll, yaw, temp;

    M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    M5.IMU.getTempData(&temp);

    updateOrientation(gyroX, gyroY, gyroZ, accX, accY, accZ,
                      pitch, roll, yaw, offsetX, offsetY, offsetZ);

    // M5.Lcd.setCursor(0, 20);
    // M5.Lcd.printf("gyro: %6.2f %6.2f %6.2f\n", gyroX, gyroY, gyroZ);
    // M5.Lcd.setCursor(0, 70);
    // M5.Lcd.printf("acc : %5.2f %5.2f %5.2f\n", accX, accY, accZ);
    // M5.Lcd.setCursor(0, 120);
    // M5.Lcd.printf("angle: %5.2f %5.2f %5.2f\n", pitch, roll, yaw);
    // M5.Lcd.setCursor(0, 175);
    // M5.Lcd.printf("Temp : %.2f C", temp);

    String data = String(millis()) + "," +
                  String(accX, 6) + "," + String(accY, 6) + "," + String(accZ, 6) + "," +
                  String(gyroX, 6) + "," + String(gyroY, 6) + "," + String(gyroZ, 6) + "," +
                  String(pitch, 6) + "," + String(roll, 6) + "," + String(yaw, 6) + "," +
                  String(temp, 2);
    dataBuffer.push_back(data);

    if (dataBuffer.size() >= BUFFER_SIZE) {
        writeDataBuffered(dataBuffer, CSV_FILENAME);
        dataBuffer.clear();
    }


    // ─────────────────────────────
    // Serial Communication
    // ─────────────────────────────
    if (Serial.available() >= 6) {
        uint8_t header = Serial.read();
        if (header != '#') return;

        // Reading the packet
        uint8_t device_id = Serial.read();
        uint8_t target_id = Serial.read();
        uint8_t command = Serial.read();
        uint8_t crc_recv = Serial.read();
        uint8_t footer = Serial.read();

        // debug output
        M5.Lcd.setCursor(0, 20);
        M5.Lcd.printf("dev: %d, tar: %d, com: %d, crc: %d\n", device_id, target_id, command, crc_recv);

        if (footer != '\r') return;

        // Calculate CRC
        uint8_t crc_input[] = {device_id, target_id, command};
        uint8_t crc_calc = crc8_calculate(crc_input, 3);
        M5.Lcd.setCursor(0, 50);
        M5.Lcd.printf("crc_calc: %d\n", crc_calc);

        if (crc_calc == crc_recv) {
            // ─────────────────────────────
            // 応答送信（形式: own_id, status, crc8, \r）
            // ─────────────────────────────
            uint8_t own_id = target_id;  // 応答元（M5Stack）のID
            uint8_t status = 0x55;       // 任意のステータス（成功の意など）
            uint8_t response[2] = {own_id, status};
            uint8_t crc = crc8_calculate(response, 2);

            Serial.write(own_id);
            Serial.write(status);
            Serial.write(crc);
            Serial.write('\r');
        } else {
            M5.Lcd.setCursor(0, 220);
            M5.Lcd.printf("❌ CRC Mismatch!\n");
        }
    }

    delay(10);  // 約100Hz送信
}
