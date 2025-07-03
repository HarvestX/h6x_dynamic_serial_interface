// Copyright 2025 HarvestX Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef H6X_DYNAMIC_SERIAL_PORT_HANDLER_HPP
#define H6X_DYNAMIC_SERIAL_PORT_HANDLER_HPP

#include <atomic>
#include <functional>
#include <thread>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <string>
#include <libserial/SerialPort.h>

#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_crc8.h"
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_handler.h"
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_definitions_base.h"
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_big_endian.h"

namespace h6x_dynamic_serial_port_handler
{

class SerialPortHandler
{
private:
  std::unique_ptr<LibSerial::SerialPort> serial_port;
  std::atomic<bool> running;
  std::thread timer_thread;
  std::function<void(const Packet &)> data_callback;
  int timer_interval_ms;
  mutable std::mutex serial_mutex;

public:
  SerialPortHandler();
  ~SerialPortHandler();
  bool init_serial(const std::string &, const int);

  bool put_serial_data(const Packet *);
  bool get_serial_data(Packet *, const uint8_t);

  void print_packet_bytes(const char *, size_t);
  void set_data_callback(std::function<void(const Packet &)>);
  bool pub_sub(const Packet &, Packet &);
};

}  // namespace h6x_dynamic_serial_port_handler

#endif  // H6X_DYNAMIC_SERIAL_PORT_HANDLER_HPP
