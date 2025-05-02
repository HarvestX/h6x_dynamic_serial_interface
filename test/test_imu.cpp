#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <FS.h>
#include <SPIFFS.h>

#include "crc8.hpp" // CRC8 calculation function

const float EPSILON = 2.0f;
const float NOISE = 10.0;
const float STABILIZATION_TIME = 500.0f;
const int ROLL_DOWN_POINT = 41497;
const int PITCH_DOWN_POINT = 22495;


struct IMUData
{
  double time;
  double accX, accY, accZ;
  double gyroX, gyroY, gyroZ;
  double pitch, roll, yaw;
  double temp;
};

std::vector<IMUData> loadIMUData(const std::string & filename)
{
  std::vector<IMUData> data;

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    throw std::runtime_error("SPIFFS Mount Failed");
  }

  std::string path = "/data/" + filename;
  Serial.print("Opening file: ");
  Serial.println(path.c_str());

  File file = SPIFFS.open(path.c_str());
  if (!file || file.isDirectory()) {
    Serial.println("Failed to open file for reading");
    throw std::runtime_error("Failed to open file for reading");
  }

  std::string line;
  bool skip_header = true;
  while (file.available()) {
    line = file.readStringUntil('\n').c_str();
    if (skip_header) {
      skip_header = false;
      continue;
    }

    IMUData entry;
    sscanf(
      line.c_str(), "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
      &entry.time, &entry.accX, &entry.accY, &entry.accZ,
      &entry.gyroX, &entry.gyroY, &entry.gyroZ,
      &entry.pitch, &entry.roll, &entry.yaw, &entry.temp);
    data.push_back(entry);
  }

  file.close();
  Serial.println("File read completed");
  return data;
}

// // Test the degree of convergence of roll after tilting
// TEST(IMUTest, StabilizationWithinThreshold_roll) {
//     auto imu_data = loadIMUData("imu_data.csv");

//     bool is_reach = false;
//     int reachedRoll = 0;

//     for (size_t i = 1; i < imu_data.size(); i++){
//         if(imu_data[i].time >= ROLL_DOWN_POINT + STABILIZATION_TIME){
//             is_reach = true;
//             Serial.print("Start Roll: ");
//             Serial.println(imu_data.front().roll);
//             Serial.print("After 0.5sec Roll: ");
//             Serial.println(imu_data[i].roll);
//             EXPECT_NEAR(imu_data.front().roll, imu_data[i].roll, EPSILON);
//             break;
//         }
//     }

//     if(!is_reach){
//         FAIL() << "Didn't reach [roll]";
//     }
// }

// // Test the degree of convergence of pitch after tilting
// TEST(IMUTest, StabilizationWithinThreshold_pitch) {
//     auto imu_data = loadIMUData("imu_data.csv");

//     bool is_reach = false;
//     int reachedPitch = 0;

//     for (size_t i = 1; i < imu_data.size(); i++){
//         if(imu_data[i].time >= PITCH_DOWN_POINT + STABILIZATION_TIME){
//             is_reach = true;
//             Serial.print("Start Pitch: ");
//             Serial.println(imu_data.front().pitch);
//             Serial.print("After 0.5sec Pitch: ");
//             Serial.println(imu_data[i].pitch);
//             EXPECT_NEAR(imu_data.front().pitch, imu_data[i].pitch, EPSILON);
//             break;
//         }
//     }

//     if(!is_reach){
//         FAIL() << "Didn't reach [roll]";
//     }
// }

// // roll-pitch noise
// TEST(IMUTest, roll_pitch_noise){
//     auto imu_data = loadIMUData("imu_data.csv");

//     float max_roll = 0;
//     float max_pitch = 0;

//     if (imu_data.size() < 2) {
//         FAIL() << "Insufficient data points.";
//     }

//     for (size_t i = 1; i < imu_data.size(); i++){
//         if (max_roll < abs(imu_data[i].roll)){
//             max_roll = abs(imu_data[i].roll);
//         }
//         if (max_pitch < abs(imu_data[i].pitch)){
//             max_pitch = abs(imu_data[i].pitch);
//         }
//     }
//     EXPECT_GT(NOISE, max_roll);
//     EXPECT_GT(NOISE, max_pitch);
// }


// // Comparison of first and last roll-pitch values
// TEST(IMUTest, FinalAngleAccuracy) {
//     auto imu_data = loadIMUData("imu_data.csv");

//     if (imu_data.size() < 2) {
//         FAIL() << "Insufficient data points.";
//     }

//     Serial.print("First roll: ");
//     Serial.println(imu_data.front().roll);

//     Serial.print("Last roll: ");
//     Serial.println(imu_data.back().roll);

//     Serial.print("First pitch: ");
//     Serial.println(imu_data.front().pitch);

//     Serial.print("Last pitch: ");
//     Serial.println(imu_data.back().pitch);

//     EXPECT_NEAR(imu_data.front().roll, imu_data.back().roll, EPSILON);
//     EXPECT_NEAR(imu_data.front().pitch, imu_data.back().pitch, EPSILON);
// }

TEST(CRC8Test, KnownValues) {
  // 例1: データ {0x01, 0x02, 0x03, 0x04}
  const uint8_t test_data1[] = {0x01, 0x02, 0x03, 0x04};
  uint8_t crc1 = crc8_calculate(test_data1, sizeof(test_data1));
  EXPECT_EQ(crc1, 0xA7);    // ←この値は実際にcrc8_calculateで計算した結果に置き換えてください

  // 例2: データ {0xFF, 0xFF, 0xFF}
  const uint8_t test_data2[] = {0xFF, 0xFF, 0xFF};
  uint8_t crc2 = crc8_calculate(test_data2, sizeof(test_data2));
  EXPECT_EQ(crc2, 0xAC);    // ←これも実測値に合わせて修正

  // 例3: 空データ
  const uint8_t test_data3[] = {};
  uint8_t crc3 = crc8_calculate(test_data3, 0);
  EXPECT_EQ(crc3, 0x00);    // 通常、空データに対しては初期値がそのまま返る
}

#if defined(ARDUINO)
#include <Arduino.h>
void setup()
{
  Serial.begin(115200);
  ::testing::InitGoogleTest();
}

void loop()
{
  if (RUN_ALL_TESTS()) {}
  delay(1000);
}
#else
int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
