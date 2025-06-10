#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <string.h>
#include <stdint.h>
#include <math.h>
#include "protocol_definitions.h"
#include "crc8.h"
#include "imu_filter.h"
#include "cpu_usage_handler.hpp"

#define CALIBRATION_TIME 200  // Number of cycles for gyro calibration

// Global variables
uint8_t calibration_count = 0;
float roll = 0.0f, pitch = 0.0f, yaw = 0.0f, temp = 0.0f;
float offsetX = 0, offsetY = 0, offsetZ = 0;
unsigned long prev_time = millis();  // Timestamp for delta time calculation

// Low-pass filter to smooth values
float lowPassFilter(const float current_value, const float previous_value, const float alpha)
{
  return alpha * current_value + (1.0f - alpha) * previous_value;
}

// Update Roll, Pitch, Yaw from IMU data
uint8_t updateRPYFromIMU(float dt, float * roll, float * pitch, float * yaw)
{
  static bool calibrated = false;
  static float gyroZ_offset = 0.0f;
  static float prev_accX = 0.0f, prev_accY = 0.0f, prev_accZ = 0.0f;
  static float prev_gyroX = 0.0f, prev_gyroY = 0.0f, prev_gyroZ = 0.0f;

  float accX, accY, accZ;
  float gyroX, gyroY, gyroZ;
  float local_temp;

  // Get raw sensor data
  M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);   // [deg/s]
  M5.IMU.getAccelData(&accX, &accY, &accZ);     // [g]
  M5.IMU.getTempData(&::temp);                  // Temperature

  // Update orientation using external filter
  updateOrientation(
    gyroX, gyroY, gyroZ, accX, accY, accZ, *pitch, *roll, *yaw, offsetX, offsetY,
    offsetZ);

  return 0; // Success
}

// Convert uint32_t to big-endian byte array
void big_endian(const uint32_t value, uint8_t * out_array, uint8_t * out_len)
{
  int byte_count = 0;
  for (int shift = 24; shift >= 0; shift -= 8) {
    uint8_t byte = (value >> shift) & 0xFF;
    if (byte_count == 0 && byte == 0) {continue;}
    out_array[byte_count++] = byte;
  }

  if (byte_count == 0) {
    out_array[0] = 0x00;
    byte_count = 1;
  }

  *out_len = byte_count;
}

// Convert date string (e.g., "2025/04/25") to ASCII byte array
void convert_date_to_ascii_array(const char * date_str, uint8_t * ascii_array, uint8_t * data_len, const uint8_t max_array_size)
{
  if (!date_str || !ascii_array || !data_len) {return;}

  uint8_t len = strnlen(date_str, max_array_size);
  for (uint8_t i = 0; i < len; ++i) {
    ascii_array[i] = static_cast<uint8_t>(date_str[i]);
  }
  M5.Lcd.println();
  *data_len = len;
}


void command_handler(const uint8_t & command, Packet & s_pkt)
{
  s_pkt.status = ERR_SUCCESS;
  switch (command) {
    case CMD_PING: {
        uint8_t send_data[] = {0x00};
        memcpy(s_pkt.data, send_data, sizeof(send_data));
        s_pkt.data_len = sizeof(send_data);
        break;
      }
    case CMD_INTERNAL_LED_ON_OFF: {
        uint8_t send_data[] = {0x00};
        memcpy(s_pkt.data, send_data, sizeof(send_data));
        s_pkt.data_len = sizeof(send_data);
        break;
      }
    case CMD_REBOOT_DEVICE: {
        uint8_t send_data[] = {0x00};
        memcpy(s_pkt.data, send_data, sizeof(send_data));
        s_pkt.data_len = sizeof(send_data);
        ESP.restart();   // Reboot the ESP32 device
        break;
      }
    case CMD_REQUEST_GENERAL_STATUS: {
        uint8_t send_data[] = {0x05, VERSION};
        memcpy(s_pkt.data, send_data, sizeof(send_data));
        s_pkt.data_len = sizeof(send_data);
        break;
      }
    case CMD_REQUEST_DEVICE_TICK: {
        s_pkt.elapsed_tick = millis() - s_pkt.start_tick;
        uint8_t send_data[4];
        uint8_t data_len;
        big_endian(s_pkt.elapsed_tick, send_data, &data_len);
        memcpy(s_pkt.data, send_data, data_len);
        s_pkt.data_len = data_len;
        break;
      }
    case CMD_REQUEST_INTERNAL_ID: {
        uint8_t send_data[] = {0x10, 0x11, 0x12, 0x13};
        memcpy(s_pkt.data, send_data, sizeof(send_data));
        s_pkt.data_len = sizeof(send_data);
        break;
      }
    case CMD_REQUEST_FIRMWARE_WRITE_DATE: {
        uint8_t send_data[16];
        uint8_t data_len = 0;
        const char* message = "2025/04/25";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &data_len, message_len);
        memcpy(s_pkt.data, send_data, data_len);
        s_pkt.data_len = data_len;
        break;
      }
    case CMD_REQUEST_DEVICE_VENDOR: {
        uint8_t send_data[16];
        uint8_t data_len = 0;
        const char* message = "Espressif";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &data_len, message_len);
        break;
      }
    case CMD_REQUEST_DEVICE_NAME: {
        uint8_t send_data[16];
        uint8_t data_len = 0;
        const char* message = "ESP32";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &data_len, message_len);
        break;
      }
    case CMD_REQUEST_CURRENT_STATE: {
        uint8_t send_data[] = {0x0F};
        break;
      }
    case CMD_REQUEST_IMU: {
        if (s_pkt.mode == 0){
          unsigned long now = millis();
          float dt = (now - prev_time) / 1000.0f;
          prev_time = now;
          // Update RPY values from IMU
          uint8_t status = updateRPYFromIMU(dt, &roll, &pitch, &yaw);
          uint8_t send_data[16];
          memcpy(send_data, &roll, sizeof(float));
          memcpy(send_data + 4, &pitch, sizeof(float));
          memcpy(send_data + 8, &yaw, sizeof(float));
          memcpy(send_data + 12, &temp, sizeof(float));

          s_pkt.data_len = sizeof(send_data);
          memcpy(s_pkt.data, send_data, sizeof(send_data));
  
          s_pkt.status = status;
        }
        else if(s_pkt.mode == 1){
          float usage = getCPUUsage();
          uint8_t send_data[4];
          encodeCPUUsage(usage, send_data);
          s_pkt.data_len = sizeof(send_data);
          memcpy(s_pkt.data, send_data, sizeof(send_data));
        }
        break;
      }
    case CMD_REQUEST_CARRIPLATION_STATUS: {
        uint8_t status;
        if (calibration_count >= CALIBRATION_TIME) {
          status = 0x02;
        } else if (calibration_count > 0) {
          status = 0x01;
        } else {
          status = 0x00;
        }
        uint8_t send_data[] = {0x00};
        s_pkt.data_len = sizeof(send_data);
        memcpy(s_pkt.data, send_data, sizeof(send_data));
        s_pkt.status = status;
        break;
      }
    case CMD_REQUEST_CARRIPLATION_EXECUSION: {
        uint8_t status = ERR_SUCCESS;
        if (calibration_count < CALIBRATION_TIME) {
          calcCalibrationOffset(&offsetX, &offsetY, &offsetZ);
          float gx, gy, gz;
          M5.IMU.getGyroData(&gx, &gy, &gz);
          updateCalibration(gx, gy, gz);
          calibration_count++;
          status = NOW_CALIBRATING;
        }

        uint8_t send_data[] = {0x00};
        s_pkt.data_len = sizeof(send_data);
        memcpy(s_pkt.data, send_data, sizeof(send_data));
        s_pkt.status = status;
        break;
      }
    case CMD_REQUEST_OUTPUT_RANDOM_NUMBER: {
        uint32_t rand_number = random(10000000, 100000000);
        char message[13];
        snprintf(message, sizeof(message), "rnd_%08lu", (unsigned long)rand_number);
        uint8_t send_data[16];
        uint8_t data_len = 0;
        convert_date_to_ascii_array(message, send_data, &data_len, sizeof(send_data));
        break;
      }

    default: {
        break;
      }
  }
  
  uint8_t response[3 + s_pkt.data_len];
  response[0] = s_pkt.header;
  response[1] = s_pkt.target_id;
  response[2] = s_pkt.status;
  response[3] = s_pkt.data_len;
  memcpy(response + 4, s_pkt.data, s_pkt.data_len);
  s_pkt.crc = crc8_calculate(response, 3 + s_pkt.data_len);
}

#endif
