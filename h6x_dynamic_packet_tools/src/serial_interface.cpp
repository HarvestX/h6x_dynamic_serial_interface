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

serialInterface::~serialInterface()
{
  std::lock_guard<std::mutex> lock(serial_mutex);
  running = false;
  
  if (timer_thread.joinable()) {
    timer_thread.join();
  }
  
  if (serial_port && serial_port->IsOpen()) {
    try {
      serial_port->Close();
    } catch (const std::exception& e) {
      std::cerr << "Error closing serial port: " << e.what() << std::endl;
    }
  }
  
  std::cout << "Serial interface destroyed" << std::endl;
}

bool serialInterface::init_serial(const std::string & port, const int baudrate = 9600)
{
  std::lock_guard<std::mutex> lock(serial_mutex);
  
  try {
    // Clean up existing connection if any
    if (serial_port && serial_port->IsOpen()) {
      serial_port->Close();
    }
    
    serial_port = std::make_unique<LibSerial::SerialPort>();
    serial_port->Open(port);
    serial_port->SetBaudRate(LibSerial::BaudRate::BAUD_9600);
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

bool serialInterface::put_serial_data(const Packet * pkt)
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

bool serialInterface::get_serial_data(Packet * recv_pkt, const uint8_t target_header)
{
  if (!serial_port || !recv_pkt) {return false;}

  try {
    char response[256];
    char c;
    size_t response_len = 0;
    int header_search_count = 0;
    const int MAX_HEADER_SEARCH = 1000;

    // Search for header byte with limited attempts
    header_search_count = 0;
    while (header_search_count < MAX_HEADER_SEARCH) {
      try {
        serial_port->ReadByte(c, 500);  // Longer timeout for header
        header_search_count++;
        printf("0x%02X ", static_cast<uint8_t>(c));
        printf("\n");
        
        if (static_cast<uint8_t>(c) == target_header) {
          response_len = 0;  // Reset response length on new header
          response[response_len++] = c;
          std::cout << "Header found: 0x" << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(target_header) << " ";
          break;
        }
      } catch (const LibSerial::ReadTimeout&) {
        std::cout << "Timeout waiting for header byte 0x" << std::hex 
                  << static_cast<int>(target_header) << std::endl;
        return false;
      } catch (const std::exception& e) {
        std::cerr << "Serial read error: " << e.what() << std::endl;
        return false;
      }
    }
    
    
    if (header_search_count >= MAX_HEADER_SEARCH) {
      std::cout << "Header search exceeded maximum attempts" << std::endl;
      return false;
    }

    // Read the remaining header bytes (client_id, command, data_length)
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
      std::cout << "Packet length: " << static_cast<int>(packet_length) << " ";
    } catch (const LibSerial::ReadTimeout&) {
      std::cout << "Timeout reading packet header" << std::endl;
      return false;
    } catch (const std::exception& e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }
    
    // Validate packet length
    if (packet_length > 200 || response_len + packet_length + 2 > 255) {
      std::cout << "Invalid packet length: " << static_cast<int>(packet_length) << std::endl;
      return false;
    }
    
    // Read data bytes
    try {
      for (size_t i = 0; i < packet_length; i++) {
        serial_port->ReadByte(c, 200);
        response[response_len++] = c;
      }
    } catch (const LibSerial::ReadTimeout&) {
      std::cout << "Timeout reading packet data" << std::endl;
      return false;
    } catch (const std::exception& e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }
    
    // Read the CRC byte and end character
    try {
      serial_port->ReadByte(c, 200);
      response[response_len++] = c;
    } catch (const LibSerial::ReadTimeout&) {
      std::cout << "Timeout reading CRC or end character" << std::endl;
      return false;
    } catch (const std::exception& e) {
      std::cerr << "Serial read error: " << e.what() << std::endl;
      return false;
    }

    std::cout << std::endl;
    print_packet_bytes(response, response_len);

    // Initialize packet structure
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

void serialInterface::print_packet_bytes(const char * packet, size_t length)
{
  std::cout << "->: ";
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
  std::lock_guard<std::mutex> lock(serial_mutex);
  
  // Initialize response packet
  memset(&recv_pkt, 0, sizeof(Packet));
  recv_pkt.is_valid = false;
  
  if (!put_serial_data(&pkt)) {
    std::cout << "Failed to send packet" << std::endl;
    return false;
  }
  
  std::cout << "Packet sent, waiting for response..." << std::endl;
  // Reduced wait time for faster response
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
  std::lock_guard<std::mutex> lock(serial_mutex);
  
  if (!serial_port) {
    std::cout << "Serial port not initialized" << std::endl;
    return false;
  }

  // Initialize packet
  memset(&recv_pkt, 0, sizeof(Packet));
  recv_pkt.is_valid = false;
  
  // Read data into recv_pkt
  if (get_serial_data(&recv_pkt, 0x23)) {
    if (data_callback) {
      data_callback(recv_pkt);
    }
    return true;
  } else {
    // Don't log failure as it's expected during polling
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
