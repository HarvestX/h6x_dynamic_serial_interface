#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_PREFIX="${1:-/usr/local}"
TARGET="${2:-cortex-m0plus}"

if [[ "$TARGET" == "stm32f4" ]]; then
    echo "Building h6x_dynamic_packet_handler for ARM Cortex-M4 (STM32F4 with FPU)..."
    MAKE_TARGET="STM32F4=1"
    LIB_NAME="libh6x_dynamic_packet_handler_stm32f4.a"
else
    echo "Building h6x_dynamic_packet_handler for ARM Cortex-M (32-bit)..."
    MAKE_TARGET="release"
    LIB_NAME="libh6x_dynamic_packet_handler.a"
fi

if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "Error: arm-none-eabi-gcc not found. Please install ARM embedded toolchain."
    echo "On Ubuntu/Debian: sudo apt-get install gcc-arm-none-eabi"
    exit 1
fi

make clean

make ${MAKE_TARGET}

mkdir -p "${INSTALL_PREFIX}/lib/arm-none-eabi"

cp build/libh6x_dynamic_packet_handler.a "${INSTALL_PREFIX}/lib/arm-none-eabi/${LIB_NAME}"

echo "ARM Cortex-M build completed successfully!"
echo "Library installed to: ${INSTALL_PREFIX}/lib/arm-none-eabi/${LIB_NAME}"