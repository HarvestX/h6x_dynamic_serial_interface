/*
 * Copyright (c) 2025 HarvestX Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include "serial_interface.hpp"

serialInterface::serialInterface()
: running(false), timer_interval_ms(1000) {}

bool serialInterface::init_serial(const std::string & port, const int baudrate = 9600)
{
  try {
    serial_stream = std::make_unique<LibSerial::SerialStream>();
    serial_stream->Open(port);
    serial_stream->SetBaudRate(LibSerial::BaudRate::BAUD_9600);
    serial_stream->SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
    serial_stream->SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
    serial_stream->SetParity(LibSerial::Parity::PARITY_NONE);
    serial_stream->SetStopBits(LibSerial::StopBits::STOP_BITS_1);
    return true;
  } catch (const std::exception & e) {
    std::cerr << "Serial initialization error: " << e.what() << std::endl;
    return false;
  }
}

bool serialInterface::put_serial_data(const Packet * pkt)
{
  if (!serial_stream) {return false;}

  char send_packet[256];
  if (!create_packet(pkt, send_packet)) {
    std::cout << "Failed to create packet" << std::endl;
    return false;
  }

  size_t packet_length = pkt->data_len + 6;

  try {
    for (size_t i = 0; i < packet_length; i++) {
      *serial_stream << send_packet[i];
    }

    std::cout << "->: ";
    print_packet_bytes(send_packet, packet_length);
    return true;
  } catch (const std::exception & e) {
    std::cerr << "Serial send error: " << e.what() << std::endl;
    return false;
  }
}

bool serialInterface::get_serial_data(Packet * recv_pkt)
{
  if (!serial_stream) {return false;}

  try {
    char response[256];
    char c;
    size_t response_len = 0;

    auto start_time = std::chrono::steady_clock::now();
    while (1) {
      if (!serial_stream->get(c)) {
        continue;
      }
      if (c == 0x24) {
        response[response_len++] = c;
        break;
      }

      auto elapsed = std::chrono::steady_clock::now() - start_time;
      if (elapsed > std::chrono::seconds(1)) {
        return false;         // Timeout
      }
    }

    uint8_t packet_length = 0;
    while (serial_stream->get(c)) {
      response[response_len++] = c;
      std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(static_cast<uint8_t>(c)) << " ";
      if (response_len == 4) {       // After reading the first 4 bytes
        packet_length = static_cast<uint8_t>(c);         // 0x0d
        break;
      }
    }
    for (size_t i = 0; i < packet_length; i++) {     // -1 because last byte is CRC
      if (!serial_stream->get(c)) {
        return false;
      }
      response[response_len++] = c;
    }
    // Read the CRC byte
    if (!serial_stream->get(c)) {
      std::cout << "Failed to read CRC byte." << std::endl;
      return false;       // Failed to read CRC byte
    }
    response[response_len++] = c;
    // read \r
    if (!serial_stream->get(c)) {
      std::cout << "Failed to read end of packet character." << std::endl;
      return false;       // Failed to read end of packet character
    }
    response[response_len++] = c;

    print_packet_bytes(response, response_len);

    if (packet_division(recv_pkt, response, response_len)) {
      // check_crc(recv_pkt) ? return true : false;
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
  if (put_serial_data(&pkt)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (get_serial_data(&recv_pkt)) {
      if (data_callback) {
        data_callback(recv_pkt);
        return true;
      }
    }
  }
  return false;
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
