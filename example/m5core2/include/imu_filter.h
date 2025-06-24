#ifndef IMU_FILTER_H
#define IMU_FILTER_H

#include <stdint.h>
#include <math.h>


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
  float & roll, float & yaw, float offsetX, float offsetY, float offsetZ,
  float dt = 0.01f) // 時間差分(デフォルト値0.01秒)
{
  gx -= offsetX;
  gy -= offsetY;
  gz -= offsetZ;
  // 単純な加速度センサ値からroll, pitchを算出
  roll = atan2(ay, az) * 180.0f / M_PI;
  pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / M_PI;
  // gyroのz軸の値からyawを更新
  yaw += gz * dt * 180.0f / M_PI; // ラジアン/秒から度に変換して積分

  // yawを-180〜180度の範囲に正規化
  while (yaw > 180.0f) {yaw -= 360.0f;}
  while (yaw < -180.0f) {yaw += 360.0f;}
}

#endif // IMU_FILTER_H
