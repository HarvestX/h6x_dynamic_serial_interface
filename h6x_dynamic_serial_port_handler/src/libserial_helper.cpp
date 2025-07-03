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

#include "h6x_dynamic_serial_port_handler/libserial_helper.hpp"

namespace h6x_dynamic_serial_port_handler
{
LibSerial::BaudRate getBaudrate(const int baud) noexcept
{
  using LibSerial::BaudRate;
  switch (baud) {
    case 50:
      return BaudRate::BAUD_50;
    case 75:
      return BaudRate::BAUD_75;
    case 110:
      return BaudRate::BAUD_110;
    case 134:
      return BaudRate::BAUD_134;
    case 150:
      return BaudRate::BAUD_150;
    case 200:
      return BaudRate::BAUD_200;
    case 300:
      return BaudRate::BAUD_300;
    case 600:
      return BaudRate::BAUD_600;
    case 1200:
      return BaudRate::BAUD_1200;
    case 1800:
      return BaudRate::BAUD_1800;
    case 2400:
      return BaudRate::BAUD_2400;
    case 4800:
      return BaudRate::BAUD_4800;
    case 9600:
      return BaudRate::BAUD_9600;
    case 19200:
      return BaudRate::BAUD_19200;
    case 38400:
      return BaudRate::BAUD_38400;
    case 57600:
      return BaudRate::BAUD_57600;
    case 115200:
      return BaudRate::BAUD_115200;
    case 230400:
      return BaudRate::BAUD_230400;
#ifdef __linux__
    case 460800:
      return BaudRate::BAUD_460800;
    case 500000:
      return BaudRate::BAUD_500000;
    case 576000:
      return BaudRate::BAUD_576000;
    case 921600:
      return BaudRate::BAUD_921600;
    case 1000000:
      return BaudRate::BAUD_1000000;
    case 1152000:
      return BaudRate::BAUD_1152000;
    case 1500000:
      return BaudRate::BAUD_1500000;
#if __MAX_BAUD > B2000000
    case 2000000:
      return BaudRate::BAUD_2000000;
    case 2500000:
      return BaudRate::BAUD_2500000;
    case 3000000:
      return BaudRate::BAUD_3000000;
    case 3500000:
      return BaudRate::BAUD_3500000;
    case 4000000:
      return BaudRate::BAUD_4000000;
#endif /* __MAX_BAUD */
#endif /* __linux__ */
  }

  return BaudRate::BAUD_INVALID;
}
}  // namespace h6x_dynamic_serial_port_handler
