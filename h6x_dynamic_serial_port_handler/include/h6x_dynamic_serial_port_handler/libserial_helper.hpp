/*
 * Copyright (c) 2025 HarvestX Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef H6X_DYNAMIC_SERIAL_PORT_HANDLER_LIBSERIAL_HELPER_HPP_
#define H6X_DYNAMIC_SERIAL_PORT_HANDLER_LIBSERIAL_HELPER_HPP_

#include <libserial/SerialStream.h>

namespace h6x_dynamic_serial_port_handler
{
LibSerial::BaudRate getBaudrate(const int baud) noexcept;
}  // namespace h6x_dynamic_serial_port_handler
#endif
