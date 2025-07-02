/*
 * Copyright (c) 2025 HarvestX Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include "serial_interface.hpp"
#include <chrono>
#include <thread>

serialInterface::serialInterface()
: running(false), timer_interval_ms(1000) {}

bool serialInterface::init_serial(const std::string & port, const int baudrate = 9600)
{
  try {
    serial_port = std::make_unique<LibSerial::SerialPort>();
    serial_port->Open(port);
    serial_port->SetBaudRate(LibSerial::BaudRate::BAUD_9600);
    serial_port->SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
    serial_port->SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
    serial_port->SetParity(LibSerial::Parity::PARITY_NONE);
    serial_port->SetStopBits(LibSerial::StopBits::STOP_BITS_1);
    return true;
  } catch (const std::exception & e) {
    std::cerr << "Serial initialization error: " << e.what() << std::endl;
    return false;
  }
}

bool serialInterface::put_serial_data(const Packet * pkt)
{
  if (!serial_port) {return false;}

  char send_packet[256];
  if (!create_packet(pkt, send_packet)) {
    std::cout << "Failed to create packet" << std::endl;
    return false;
  }

  size_t packet_length = pkt->data_len + 6;

  try {
    for (size_t i = 0; i < packet_length; i++) {
      serial_port->WriteByte(send_packet[i]);
    }

    std::cout << "->: ";
    print_packet_bytes(send_packet, packet_length);
    return true;
  } catch (const std::exception & e) {
    std::cerr << "Serial send error: " << e.what() << std::endl;
    return false;
  }
}

bool serialInterface::get_serial_data(Packet * recv_pkt, const uint8_t target_header)
{
  if (!serial_port) {return false;}

  try {
    char response[256];
    char c;
    size_t response_len = 0;

    while (1) {
      try {
        serial_port->ReadByte(c, 100);
        if (static_cast<uint8_t>(c) == target_header) {
          response_len = 0;
          response[response_len++] = c;
          break;
        }
      } catch (const LibSerial::ReadTimeout&) {
        std::cout << "Timeout waiting for header byte" << std::endl;
        return false;
      } catch (const std::exception& e) {
        std::cerr << "Serial read error: " << e.what() << std::endl;
        return false;
      }
    }

    uint8_t packet_length = 0;
    try {
      for (int i = 1; i < 4; i++) {
        serial_port->ReadByte(c, 100);
        response[response_len++] = c;
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(static_cast<uint8_t>(c)) << " ";
      }
      packet_length = static_cast<uint8_t>(response[3]);
    } catch (const LibSerial::ReadTimeout&) {
      std::cout << "Timeout reading packet header" << std::endl;
      return false;
    } catch (const std::exception& e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }
    try {
      for (size_t i = 0; i < packet_length; i++) {
        serial_port->ReadByte(c, 100);
        response[response_len++] = c;
      }
    } catch (const LibSerial::ReadTimeout&) {
      std::cout << "Timeout reading packet data" << std::endl;
      return false;
    } catch (const std::exception& e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }
    // Read the CRC byte
    try {
      serial_port->ReadByte(c, 100);
      response[response_len++] = c;
      // read \r
      serial_port->ReadByte(c, 100);
      response[response_len++] = c;
    } catch (const LibSerial::ReadTimeout&) {
      std::cout << "Timeout reading CRC or end character" << std::endl;
      return false;
    } catch (const std::exception& e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }

    print_packet_bytes(response, response_len);

    if (packet_division(recv_pkt, response, response_len)) {
      if (check_crc(recv_pkt)) {
        recv_pkt->is_valid = true;
        return true;
      } else {
        recv_pkt->is_valid = false;
      }
    } else {
      return false;
    }

    return false;
  } catch (const std::exception & e) {
    std::cerr << "Serial receive error: " << e.what() << std::endl;
    return false;
  }
}

void serialInterface::print_packet_bytes(const char * packet, size_t length)
{
  for (size_t i = 0; i < length; i++) {
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(static_cast<uint8_t>(packet[i])) << " ";
  }
  std::cout << std::endl;
}

void serialInterface::set_data_callback(std::function<void(const Packet &)> callback)
{
  data_callback = callback;
}

bool serialInterface::pub_sub(const Packet & pkt, Packet & recv_pkt)
{
  if (!put_serial_data(&pkt)) {
    std::cout << "Failed to send packet" << std::endl;
    return false;
  }
  
  std::cout << "Packet sent, waiting for response..." << std::endl;
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

bool serialInterface::sub(Packet & recv_pkt)
{
  if (!serial_port) {
    std::cout << "Serial port not initialized" << std::endl;
    return false;
  }

  // Read data into recv_pkt
  if (get_serial_data(&recv_pkt, 0x23)) {
    if (data_callback) {
      data_callback(recv_pkt);
    }
    return true;
  } else {
    std::cout << "Failed to receive packet" << std::endl;
    return false;
  }
}

// === Recived packet example ===
// void serialInterface::get_status(
//     Packet& recv_pkt,
//     const uint8_t client_id,
//     const uint8_t command) {
//     Packet send_pkt = init_packet();
//     send_pkt.mode = SERIAL_MODE_HOST;
//     send_pkt.client_id = client_id;
//     send_pkt.command = command;
//     pub_sub(send_pkt, recv_pkt);
// }
