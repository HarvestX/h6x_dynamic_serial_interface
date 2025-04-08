#pragma once

void calibrateIMU(float& offsetX, float& offsetY, float& offsetZ);
void updateOrientation(float gx, float gy, float gz, float ax, float ay, float az,
                       float& pitch, float& roll, float& yaw,
                       float offsetX, float offsetY, float offsetZ);
