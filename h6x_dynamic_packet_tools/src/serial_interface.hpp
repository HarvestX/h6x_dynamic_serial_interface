/*
 * Copyright (c) 2025 HarvestX Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef SERIAL_INTERFACE_HPP
#define SERIAL_INTERFACE_HPP

#include <atomic>
#include <functional>
#include <thread>
#include <iostream>
#include <iomanip>
#include <libserial/SerialStream.h>


#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_crc8.h"
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_handler.h"
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_definitions_base.h"
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_big_endian.h"
#include "serial_interface.hpp"


class serialInterface
{
private:
  std::unique_ptr<LibSerial::SerialStream> serial_stream;
  std::atomic<bool> running;
  std::thread timer_thread;
  std::function<void(const Packet &)> data_callback;
  int timer_interval_ms;

public:
  serialInterface();

  ~serialInterface() = default;
  bool init_serial(const std::string &, const int);

  bool put_serial_data(const Packet *);
  bool get_serial_data(Packet *, const uint8_t);

  void print_packet_bytes(const char *, size_t);
  void set_data_callback(std::function<void(const Packet &)>);
  bool pub_sub(const Packet &, Packet &);
  bool sub(Packet &);
};

#endif // SERIAL_INTERFACE_HPP
