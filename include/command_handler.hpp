#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include <M5Core2.h>
#include <string.h>
#include <stdint.h>
#include <cmath>
#include "protocol_definitions.hpp"
#include "crc8.hpp"


float roll = 0.0f, pitch = 0.0f, yaw = 0.0f, temp = 0.0f;
unsigned long prev_time = millis();
bool filter = false;


float lowPassFilter(float current_value, float previous_value, float alpha) {
  return alpha * current_value + (1.0f - alpha) * previous_value;
}

uint8_t updateRPYFromIMU(float dt, float* roll, float* pitch, float* yaw, bool filter) {
  static float prev_accX = 0.0f, prev_accY = 0.0f, prev_accZ = 0.0f;
  static float prev_gyroX = 0.0f, prev_gyroY = 0.0f, prev_gyroZ = 0.0f;

  float accX, accY, accZ;
  float gyroX, gyroY, gyroZ;
  float temp;

  M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);   // [deg/s]
  M5.IMU.getAccelData(&accX, &accY, &accZ);     // [g]
  M5.IMU.getTempData(&temp);

  if (isnan(accX) || isnan(accY) || isnan(accZ)) {
    return 2; // Invalid data
  }

  const float alpha = 0.2f;

  if (filter) {
    accX = lowPassFilter(accX, prev_accX, alpha);
    accY = lowPassFilter(accY, prev_accY, alpha);
    accZ = lowPassFilter(accZ, prev_accZ, alpha);

    gyroX = lowPassFilter(gyroX, prev_gyroX, alpha);
    gyroY = lowPassFilter(gyroY, prev_gyroY, alpha);
    gyroZ = lowPassFilter(gyroZ, prev_gyroZ, alpha);
  }

  prev_accX = accX; prev_accY = accY; prev_accZ = accZ;
  prev_gyroX = gyroX; prev_gyroY = gyroY; prev_gyroZ = gyroZ;

  float gx = gyroX * (M_PI / 180.0f);
  float gy = gyroY * (M_PI / 180.0f);
  float gz = gyroZ * (M_PI / 180.0f);

  float acc_roll  = atan2(accY, accZ);
  float acc_pitch = atan2(-accX, sqrt(accY * accY + accZ * accZ));

  float gyro_roll  = *roll  + gx * dt;
  float gyro_pitch = *pitch + gy * dt;
  float gyro_yaw   = *yaw   + gz * dt;

  *roll  = 0.9f * gyro_roll  + 0.1f * acc_roll;
  *pitch = 0.9f * gyro_pitch + 0.1f * acc_pitch;
  *yaw   = gyro_yaw;

  return 0; // Success
}


void big_endian(uint32_t value, uint8_t* out_array, uint8_t* out_len) {
  int byte_count = 0;
  for (int shift = 24; shift >= 0; shift -= 8) {
      uint8_t byte = (value >> shift) & 0xFF;
      if (byte_count == 0 && byte == 0) {
          continue;
      }
      out_array[byte_count++] = byte;
  }

  if (byte_count == 0) {
      out_array[0] = 0x00;
      byte_count = 1;
  }

  *out_len = byte_count;
}

void convert_date_to_ascii_array(const char* date_str, uint8_t* ascii_array, uint8_t* length) {
  if (!date_str || !ascii_array || !length) return;

  uint8_t len = strlen(date_str);
  for (uint8_t i = 0; i < len; ++i) {
      ascii_array[i] = static_cast<uint8_t>(date_str[i]);
  }
  M5.Lcd.println();
  *length = len;
}

void concat_arrays(const uint8_t* a, uint8_t len_a,
  const uint8_t* b, uint8_t len_b,
  uint8_t* result, uint8_t* result_len)
{
  if (len_a + len_b > 245) {
      *result_len = 0;
      return;
  }

  memcpy(result, a, len_a);
  memcpy(result + len_a, b, len_b);

  *result_len = len_a + len_b;
}

uint8_t create_crc_data(ReceivedPacket& pkt, const uint8_t* data, uint8_t len, uint8_t status) {
  pkt.send_data_len = len;
  pkt.status = status;

  memcpy(pkt.send_data, data, len);

  uint8_t response_len = 3;
  uint8_t response[response_len] = {OWN_ID, pkt.status, pkt.send_data_len};
  uint8_t crc_data[pkt.send_data_len + response_len];
  std::copy(response, response + response_len, crc_data);
  std::copy(pkt.send_data, pkt.send_data + pkt.send_data_len, crc_data + response_len);
  return crc8_calculate(crc_data, pkt.send_data_len + response_len);
}

void command_handler(ReceivedPacket& pkt) {
  switch (pkt.command) {
      case CMD_PING: {
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("PING command");
          uint8_t send_data_len = 1;
          uint8_t send_data[] = {0x00};
          uint8_t status = ERR_SUCCESS;
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_INTERNAL_LED_ON_OFF: {
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("LED command");
          uint8_t send_data_len = 1;
          uint8_t send_data[] = {0x00};
          uint8_t status = ERR_SUCCESS;
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_REBOOT_DEVICE: {
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("REBOOT command");
          uint8_t send_data_len = 1;
          uint8_t send_data[] = {0x00};
          uint8_t status = ERR_SUCCESS;
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          ESP.restart();
          break;
      }
      case CMD_REQUEST_GENERAL_STATUS: {
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("STATUS & VERSION command");
          uint8_t send_data_len = 1;
          uint8_t send_data[] = {VERSION};
          uint8_t status = ERR_SUCCESS;
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_REQUEST_DEVICE_TICK: {
        M5.Lcd.setCursor(0, 200);
        M5.Lcd.printf("TICK command");
        pkt.elapsed_tick = millis() - pkt.start_tick;
        uint8_t send_data_len = 1;
        uint8_t send_data[] = {pkt.elapsed_tick};
        big_endian(pkt.elapsed_tick, send_data, &send_data_len);
        uint8_t status = ERR_SUCCESS;
        pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
        break;
      }
      case CMD_REQUEST_INTERNAL_ID: {
        M5.Lcd.setCursor(0, 200);
        M5.Lcd.printf("ID command");
        uint8_t send_data_len = 4;
        uint8_t send_data[] = {0x10, 0x11, 0x12, 0x13};
        uint8_t status = ERR_SUCCESS;
        pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
        break;
      }
      case CMD_REQUEST_FIRMWARE_WRITE_DATE: {
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("DATE command");
          uint8_t send_data_len = 0;
          uint8_t send_data[16] = {0};
          uint8_t status = ERR_SUCCESS;
          convert_date_to_ascii_array("2025/04/25", send_data, &send_data_len);
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_REQUEST_DEVICE_VENDOR:{
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("DEVICE-VENDOR command");
          uint8_t send_data_len = 0;
          uint8_t send_data[16] = {0};
          uint8_t status = ERR_SUCCESS;
          convert_date_to_ascii_array("Espressif", send_data, &send_data_len);
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_REQUEST_DEVICE_NAME:{
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("DEVICE-NAME command");
          uint8_t send_data_len = 0;
          uint8_t send_data[16] = {0};
          uint8_t status = ERR_SUCCESS;
          convert_date_to_ascii_array("ESP32", send_data, &send_data_len);
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_REQUEST_CURRENT_STATE:{
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("CURRENT-STATE command");
          uint8_t send_data_len = 4;
          uint8_t send_data[] = {0x0F};
          uint8_t status = ERR_SUCCESS;
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_REQUEST_IMU:{
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("IMU command");
          unsigned long now = millis();
          float dt = (now - prev_time) / 1000.0f;
          prev_time = now;
          uint8_t status = updateRPYFromIMU(dt, &roll, &pitch, &yaw, filter);
          uint8_t send_data_len = 16;
          uint8_t send_data[16];
          memcpy(send_data, &roll, sizeof(float));
          memcpy(send_data + 4, &pitch, sizeof(float));
          memcpy(send_data + 8, &yaw, sizeof(float));
          memcpy(send_data + 12, &temp, sizeof(float));
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_REQUEST_CARRIPLATION_STATUS:{
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("CARRIPLATION-STATUS command");
          uint8_t send_data_len = 1;
          uint8_t send_data[] = {0x00};
          uint8_t status;;
          if (filter) {
            status = 0x02;
          } else {
            status = 0x00;
          }
          
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }
      case CMD_REQUEST_CARRIPLATION_EXECUSION:{
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("CARRIPLATION-EXECUSION command");
          uint8_t send_data_len = 1;
          uint8_t send_data[] = {0x00};
          uint8_t status = ERR_SUCCESS;
          filter = true;
          pkt.crc_send = create_crc_data(pkt, send_data, send_data_len, status);
          break;
      }

      default:
          M5.Lcd.setCursor(0, 200);
          M5.Lcd.printf("Unknown command");
  }
}

#ifdef __cplusplus
}
#endif

#endif