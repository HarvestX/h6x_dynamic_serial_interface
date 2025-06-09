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

// Create CRC-8 checksum based on status and payload
uint8_t create_crc_data(ReceivedPacket & pkt)
{
  uint8_t response[3 + pkt.send_data_len];
  response[0] = pkt.header;
  response[1] = (pkt.mode == SERIAL_MODE_PRIMARY) ? pkt.target_id : pkt.device_id;
  response[2] = (pkt.mode == SERIAL_MODE_PRIMARY) ? pkt.command : pkt.status;
  response[3] = pkt.send_data_len;
  memcpy(response + 4, pkt.send_data, pkt.send_data_len);
  return crc8_calculate(response, sizeof(response));
}


void command_handler(ReceivedPacket & pkt)
{
  pkt.status = ERR_SUCCESS;
  switch (pkt.command) {
    case CMD_PING: {
        uint8_t send_data[] = {0x00};
        memcpy(pkt.send_data, send_data, sizeof(send_data));
        pkt.send_data_len = sizeof(send_data);
        
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_INTERNAL_LED_ON_OFF: {
        uint8_t send_data[] = {0x00};
        memcpy(pkt.send_data, send_data, sizeof(send_data));
        pkt.send_data_len = sizeof(send_data);
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REBOOT_DEVICE: {
        uint8_t send_data[] = {0x00};
        memcpy(pkt.send_data, send_data, sizeof(send_data));
        pkt.send_data_len = sizeof(send_data);
        pkt.crc_send = create_crc_data(pkt);
        ESP.restart();   // Reboot the ESP32 device
        break;
      }
    case CMD_REQUEST_GENERAL_STATUS: {
        uint8_t send_data[] = {0x05, VERSION};
        memcpy(pkt.send_data, send_data, sizeof(send_data));
        pkt.send_data_len = sizeof(send_data);
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REQUEST_DEVICE_TICK: {
        pkt.elapsed_tick = millis() - pkt.start_tick;
        uint8_t send_data[4];
        uint8_t send_data_len;
        big_endian(pkt.elapsed_tick, send_data, &send_data_len);
        memcpy(pkt.send_data, send_data, send_data_len);
        pkt.send_data_len = send_data_len;
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REQUEST_INTERNAL_ID: {
        uint8_t send_data[] = {0x10, 0x11, 0x12, 0x13};
        memcpy(pkt.send_data, send_data, sizeof(send_data));
        pkt.send_data_len = sizeof(send_data);
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REQUEST_FIRMWARE_WRITE_DATE: {
        uint8_t send_data[16];
        uint8_t send_data_len = 0;
        const char* message = "2025/04/25";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &send_data_len, message_len);
        memcpy(pkt.send_data, send_data, send_data_len);
        pkt.send_data_len = send_data_len;
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REQUEST_DEVICE_VENDOR: {
        uint8_t send_data[16];
        uint8_t send_data_len = 0;
        const char* message = "Espressif";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &send_data_len, message_len);
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REQUEST_DEVICE_NAME: {
        uint8_t send_data[16];
        uint8_t send_data_len = 0;
        const char* message = "ESP32";
        uint8_t message_len = strnlen(message, sizeof(send_data));
        convert_date_to_ascii_array(message, send_data, &send_data_len, message_len);
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REQUEST_CURRENT_STATE: {
        uint8_t send_data[] = {0x0F};
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REQUEST_IMU: {
        if (pkt.mode == 0){
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

          pkt.send_data_len = sizeof(send_data);
          memcpy(pkt.send_data, send_data, sizeof(send_data));
  
          pkt.status = status;
          pkt.crc_send = create_crc_data(pkt); 
        }
        else if(pkt.mode == 1){
          float usage = getCPUUsage();
          uint8_t send_data[4];
          encodeCPUUsage(usage, send_data);
          pkt.send_data_len = sizeof(send_data);
          memcpy(pkt.send_data, send_data, sizeof(send_data));
          pkt.crc_send = create_crc_data(pkt); 
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
        pkt.send_data_len = sizeof(send_data);
        memcpy(pkt.send_data, send_data, sizeof(send_data));
        pkt.status = status;
        pkt.crc_send = create_crc_data(pkt);
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
        pkt.send_data_len = sizeof(send_data);
        memcpy(pkt.send_data, send_data, sizeof(send_data));
        pkt.status = status;
        pkt.crc_send = create_crc_data(pkt);
        break;
      }
    case CMD_REQUEST_OUTPUT_RANDOM_NUMBER: {
        uint32_t rand_number = random(10000000, 100000000);
        char message[13];
        snprintf(message, sizeof(message), "rnd_%08lu", (unsigned long)rand_number);
        uint8_t send_data[16];
        uint8_t send_data_len = 0;
        convert_date_to_ascii_array(message, send_data, &send_data_len, sizeof(send_data));
        pkt.crc_send = create_crc_data(pkt);
        break;
      }

    default: {
        break;
      }
  }
}

#endif
