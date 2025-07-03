/*
 * Copyright (c) 2025 HarvestX Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "h6x_dynamic_serial_port_handler/serial_port_handler.hpp"
#include "h6x_dynamic_serial_port_handler/libserial_helper.hpp"
#include <chrono>
#include <thread>

namespace h6x_dynamic_serial_port_handler
{

SerialPortHandler::SerialPortHandler()
: running(false), timer_interval_ms(1000) {}

SerialPortHandler::~SerialPortHandler()
{
  std::lock_guard<std::mutex> lock(serial_mutex);
  running = false;

  if (timer_thread.joinable()) {
    timer_thread.join();
  }

  if (serial_port && serial_port->IsOpen()) {
    try {
      serial_port->Close();
    } catch (const std::exception & e) {
      std::cerr << "Error closing serial port: " << e.what() << std::endl;
    }
  }

  std::cout << "Serial port handler destroyed" << std::endl;
}

bool SerialPortHandler::init_serial(const std::string & port, const int baudrate = 9600)
{
  std::lock_guard<std::mutex> lock(serial_mutex);

  try {
    if (serial_port && serial_port->IsOpen()) {
      serial_port->Close();
    }

    serial_port = std::make_unique<LibSerial::SerialPort>();
    serial_port->Open(port);
    serial_port->SetBaudRate(getBaudrate(baudrate));
    serial_port->SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
    serial_port->SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
    serial_port->SetParity(LibSerial::Parity::PARITY_NONE);
    serial_port->SetStopBits(LibSerial::StopBits::STOP_BITS_1);

    serial_port->FlushInputBuffer();
    serial_port->FlushOutputBuffer();

    std::cout << "Serial port initialized: " << port << std::endl;
    return true;
  } catch (const std::exception & e) {
    std::cerr << "Serial initialization error: " << e.what() << std::endl;
    return false;
  }
}

bool SerialPortHandler::put_serial_data(const Packet * pkt)
{
  if (!serial_port || !pkt) {return false;}

  char send_packet[256];
  if (!create_packet(pkt, send_packet)) {
    std::cout << "Failed to create packet" << std::endl;
    return false;
  }

  size_t packet_length = pkt->data_len + ADDITIONAL_PACKET_LENGTH;

  if (packet_length > 255) {
    std::cout << "Packet too large: " << packet_length << std::endl;
    return false;
  }

  try {
    serial_port->Write(std::string(send_packet, packet_length));

    print_packet_bytes(send_packet, packet_length);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    return true;
  } catch (const std::exception & e) {
    std::cerr << "Serial send error: " << e.what() << std::endl;
    return false;
  }
}

bool SerialPortHandler::get_serial_data(Packet * recv_pkt, const uint8_t target_header)
{
  if (!serial_port || !recv_pkt) {return false;}

  try {
    char response[256];
    char c;
    size_t response_len = 0;
    int header_search_count = 0;
    const int MAX_HEADER_SEARCH = 1000;

    header_search_count = 0;
    while (header_search_count < MAX_HEADER_SEARCH) {
      try {
        serial_port->ReadByte(c, 500);
        header_search_count++;
        printf("0x%02X ", static_cast<uint8_t>(c));
        printf("\n");

        if (static_cast<uint8_t>(c) == target_header) {
          response_len = 0;
          response[response_len++] = c;
          std::cout << "Header found: 0x" << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(target_header) << " ";
          break;
        }
      } catch (const LibSerial::ReadTimeout &) {
        return false;
      } catch (const std::exception & e) {
        std::cerr << "Serial read error: " << e.what() << std::endl;
        return false;
      }
    }

    if (header_search_count >= MAX_HEADER_SEARCH) {
      std::cout << "Header search exceeded maximum attempts" << std::endl;
      return false;
    }

    uint8_t packet_length = 0;
    try {
      for (int i = 1; i < 4; i++) {
        if (response_len >= 255) {
          std::cout << "Buffer overflow protection" << std::endl;
          return false;
        }
        serial_port->ReadByte(c, 200);
        response[response_len++] = c;
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(static_cast<uint8_t>(c)) << " ";
      }
      packet_length = static_cast<uint8_t>(response[3]);
    } catch (const LibSerial::ReadTimeout &) {
      return false;
    } catch (const std::exception & e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }

    if (packet_length > 200 || response_len + packet_length + 2 > 255) {
      std::cout << "Invalid packet length: " << static_cast<int>(packet_length) << std::endl;
      return false;
    }

    try {
      for (size_t i = 0; i < packet_length; i++) {
        serial_port->ReadByte(c, 200);
        response[response_len++] = c;
      }
    } catch (const LibSerial::ReadTimeout &) {
      std::cout << "Timeout reading packet data" << std::endl;
      return false;
    } catch (const std::exception & e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }

    try {
      serial_port->ReadByte(c, 200);
      response[response_len++] = c;
    } catch (const LibSerial::ReadTimeout &) {
      std::cout << "Timeout reading CRC or end character" << std::endl;
      return false;
    } catch (const std::exception & e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }

    std::cout << std::endl;
    print_packet_bytes(response, response_len);

    memset(recv_pkt, 0, sizeof(Packet));

    if (packet_division(recv_pkt, response, response_len)) {
      if (check_crc(recv_pkt)) {
        recv_pkt->is_valid = true;
        std::cout << "Packet received successfully" << std::endl;
        return true;
      } else {
        recv_pkt->is_valid = false;
        std::cout << "CRC check failed" << std::endl;
      }
    } else {
      std::cout << "Packet division failed" << std::endl;
      return false;
    }

    return false;
  } catch (const std::exception & e) {
    std::cerr << "Serial receive error: " << e.what() << std::endl;
    return false;
  }
}

void SerialPortHandler::print_packet_bytes(const char * packet, size_t length)
{
  std::cout << "->: ";
  for (size_t i = 0; i < length; i++) {
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(static_cast<uint8_t>(packet[i])) << " ";
  }
  std::cout << std::endl;
}

void SerialPortHandler::set_data_callback(std::function<void(const Packet &)> callback)
{
  data_callback = callback;
}

bool SerialPortHandler::pub_sub(const Packet & pkt, Packet & recv_pkt)
{
  std::lock_guard<std::mutex> lock(serial_mutex);

  memset(&recv_pkt, 0, sizeof(Packet));
  recv_pkt.is_valid = false;

  if (!put_serial_data(&pkt)) {
    std::cout << "Failed to send packet" << std::endl;
    return false;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  if (get_serial_data(&recv_pkt, 0x24)) {
    std::cout << "Response received successfully" << std::endl;
    if (data_callback) {
      data_callback(recv_pkt);
    }
    return true;
  } else {
    std::cout << "No response received (timeout)" << std::endl;
    recv_pkt.is_valid = false;
    return false;
  }
}

bool SerialPortHandler::sub(Packet & recv_pkt)
{
  std::lock_guard<std::mutex> lock(serial_mutex);

  if (!serial_port) {
    std::cout << "Serial port not initialized" << std::endl;
    return false;
  }

  memset(&recv_pkt, 0, sizeof(Packet));
  recv_pkt.is_valid = false;

  if (get_serial_data(&recv_pkt, 0x23)) {
    if (data_callback) {
      uint8_t callback_data[6] = {0x24, 0x01, 0x00, 0x01, 0x00, 0x00};
      uint8_t crc_calc = crc8_calculate(callback_data, sizeof(callback_data) - 1);
      callback_data[5] = crc_calc;
      try {
        serial_port->Write(
          std::string(
            reinterpret_cast<const char *>(callback_data),
            sizeof(callback_data)));
      } catch (const std::exception & e) {
        std::cerr << "Error sending callback data: " << e.what() << std::endl;
      }

      data_callback(recv_pkt);
    }
    return true;
  } else {
    return false;
  }
}

} // namespace h6x_dynamic_serial_port_handler
