#include <Arduino.h>
#include <M5Core2.h>
#include "imu_filter.hpp"


void calibrateIMU(float& offsetX, float& offsetY, float& offsetZ) {
    float sumX = 0, sumY = 0, sumZ = 0;
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Calibrating... Keep still");

    for (int i = 0; i < 200; i++) {
        float gx, gy, gz;
        M5.IMU.getGyroData(&gx, &gy, &gz);
        sumX += gx;
        sumY += gy;
        sumZ += gz;
        delay(10);
    }

    offsetX = sumX / 200.0f;
    offsetY = sumY / 200.0f;
    offsetZ = sumZ / 200.0f;

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Calibration Done");
}

void updateOrientation(float gx, float gy, float gz,
                       float ax, float ay, float az,
                       float& pitch, float& roll, float& yaw,
                       float offsetX, float offsetY, float offsetZ) {
    gx -= offsetX;
    gy -= offsetY;
    gz -= offsetZ;

    MahonyAHRSupdateIMU(gx * DEG_TO_RAD, gy * DEG_TO_RAD, gz * DEG_TO_RAD,
                        ax, ay, az, &pitch, &roll, &yaw);
}
