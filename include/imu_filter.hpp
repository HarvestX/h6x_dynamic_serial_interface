#include "MadgwickAHRS.h"
#include "MahonyAHRS.h"
#include <stdint.h>


static float sumX = 0, sumY = 0, sumZ = 0;
static uint32_t calibrate_sampling_count = 0;

void updateCalibration(const float gx, const float gy, const float gz)
{
  sumX += gx;
  sumY += gy;
  sumZ += gz;
  calibrate_sampling_count++;
}

void calcCalibrationOffset(float * offsetX, float * offsetY, float * offsetZ)
{
  *offsetX = sumX / calibrate_sampling_count;
  *offsetY = sumY / calibrate_sampling_count;
  *offsetZ = sumZ / calibrate_sampling_count;
}

void updateOrientation(
  float gx, float gy, float gz, float ax, float ay, float az, float & pitch,
  float & roll, float & yaw, float offsetX, float offsetY, float offsetZ)
{
  gx -= offsetX;
  gy -= offsetY;
  gz -= offsetZ;

  MahonyAHRSupdateIMU(
    gx * DEG_TO_RAD, gy * DEG_TO_RAD, gz * DEG_TO_RAD, ax, ay, az, &pitch, &roll,
    &yaw);
}
