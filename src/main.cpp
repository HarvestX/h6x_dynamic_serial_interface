#include <Arduino.h>
#include <M5Core2.h>
#include "imu_filter.hpp"
#include "data_logger.hpp"

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

    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("gyro: %6.2f %6.2f %6.2f\n", gyroX, gyroY, gyroZ);
    M5.Lcd.setCursor(0, 70);
    M5.Lcd.printf("acc : %5.2f %5.2f %5.2f\n", accX, accY, accZ);
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.printf("angle: %5.2f %5.2f %5.2f\n", pitch, roll, yaw);
    M5.Lcd.setCursor(0, 175);
    M5.Lcd.printf("Temp : %.2f C", temp);

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

    delay(10);
}