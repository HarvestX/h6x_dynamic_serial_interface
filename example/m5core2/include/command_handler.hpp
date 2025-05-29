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
void convert_date_to_ascii_array(const char * date_str, uint8_t * ascii_array, uint8_t * length, const uint8_t max_array_size)
{
  if (!date_str || !ascii_array || !length) {return;}

  uint8_t len = strnlen(date_str, max_array_size);
  for (uint8_t i = 0; i < len; ++i) {
    ascii_array[i] = static_cast<uint8_t>(date_str[i]);
  }
  M5.Lcd.println();
  *length = len;
}

// Concatenate two byte arrays into one result array
void concat_arrays(
  const uint8_t * a, const uint8_t len_a,
  const uint8_t * b, const uint8_t len_b,
  uint8_t * result, uint8_t * result_len)
{
  if (len_a + len_b > 245) {
    *result_len = 0;    // Overflow check
    return;
  }

  memcpy(result, a, len_a);
  memcpy(result + len_a, b, len_b);
  *result_len = len_a + len_b;
}

// Create CRC-8 checksum based on status and payload
uint8_t create_crc_data(ReceivedPacket & pkt, const uint8_t * data, const uint8_t len, const uint8_t status, const uint8_t mode)
{
  pkt.send_data_len = len;
  pkt.status = status;
  memcpy(pkt.send_data, data, len);

  const uint8_t response_len = (mode == 1) ? 4 : 3;
  uint8_t response[response_len];
  response[0] = OWN_ID;
  if (mode == 1) {
    response[1] = pkt.command;
  }
  response[1 + mode] = pkt.status;
  response[2 + mode] = pkt.send_data_len;

  uint8_t crc_data[pkt.send_data_len + response_len];
  std::copy(response, response + response_len, crc_data);
  std::copy(pkt.send_data, pkt.send_data + pkt.send_data_len, crc_data + response_len);

  return crc8_calculate(crc_data, pkt.send_data_len + response_len);
}



// Main command handler for all defined commands
void command_handler(ReceivedPacket & pkt, uint8_t mode)
{
  switch (pkt.command) {
    case CMD_PING: {
        uint8_t send_data[] = {0x00};
        pkt.crc_send = create_crc_data(pkt, send_data, 1, ERR_SUCCESS, mode);
        break;
      }
    case CMD_INTERNAL_LED_ON_OFF: {
        uint8_t send_data[] = {0x00};
        pkt.crc_send = create_crc_data(pkt, send_data, 1, ERR_SUCCESS, mode);
        break;
      }
    case CMD_REBOOT_DEVICE: {
        uint8_t send_data[] = {0x00};
        pkt.crc_send = create_crc_data(pkt, send_data, 1, ERR_SUCCESS, mode);
        ESP.restart();   // Reboot the ESP32 device
        break;
      }
    case CMD_REQUEST_GENERAL_STATUS: {
        uint8_t send_data[] = {0x05, VERSION};
        pkt.crc_send = create_crc_data(pkt, send_data, 2, ERR_SUCCESS, mode);
        break;
      }
    case CMD_REQUEST_DEVICE_TICK: {
        pkt.elapsed_tick = millis() - pkt.start_tick;
        uint8_t send_data[4];
        uint8_t send_data_len;
        big_endian(pkt.elapsed_tick, send_data, &send_data_len);
        pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, ERR_SUCCESS, mode);
        break;
      }
    case CMD_REQUEST_INTERNAL_ID: {
        uint8_t send_data[] = {0x10, 0x11, 0x12, 0x13};
        pkt.crc_send = create_crc_data(pkt, send_data, 4, ERR_SUCCESS, mode);
        break;
      }
    case CMD_REQUEST_FIRMWARE_WRITE_DATE: {
        uint8_t send_data[16];
        uint8_t send_data_len = 0;
        const char* message = "2025/04/25";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &send_data_len, message_len);
        pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, ERR_SUCCESS, mode);
        break;
      }
    case CMD_REQUEST_DEVICE_VENDOR: {
        uint8_t send_data[16];
        uint8_t send_data_len = 0;
        const char* message = "Espressif";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &send_data_len, message_len);
        pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, ERR_SUCCESS, mode);
        break;
      }
    case CMD_REQUEST_DEVICE_NAME: {
        uint8_t send_data[16];
        uint8_t send_data_len = 0;
        const char* message = "ESP32";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &send_data_len, message_len);
        pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, ERR_SUCCESS, mode);
        break;
      }
    case CMD_REQUEST_CURRENT_STATE: {
        uint8_t send_data[] = {0x0F};
        pkt.crc_send = create_crc_data(pkt, send_data, 1, ERR_SUCCESS, mode);
        break;
      }
    case CMD_REQUEST_IMU: {
        if (mode == 0){
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
  
          pkt.crc_send = create_crc_data(pkt, send_data, 16, status, mode); 
        }
        else if(mode == 1){
          float usage = getCPUUsage();
          uint8_t send_data[4];
          encodeCPUUsage(usage, send_data);
          pkt.crc_send = create_crc_data(pkt, send_data, 4, ERR_SUCCESS, mode); 
        }
        break;
      }
    case CMD_REQUEST_CARRIPLATION_STATUS: {
        uint8_t status;
        if (calibration_count == CALIBRATION_TIME) {
          status = 0x02;
        } else if (calibration_count > 0) {
          status = 0x01;
        } else {
          status = 0x00;
        }

        uint8_t send_data[] = {0x00};
        pkt.crc_send = create_crc_data(pkt, send_data, 1, status, mode);
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
        pkt.crc_send = create_crc_data(pkt, send_data, 1, status, mode);
        break;
      }
    default: {
        break;
      }
  }
}

#endif
