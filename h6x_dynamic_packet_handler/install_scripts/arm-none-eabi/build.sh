#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_PREFIX="${1:-/usr/local}"

echo "Building h6x_dynamic_packet_handler for ARM Cortex-M (32-bit)..."

if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "Error: arm-none-eabi-gcc not found. Please install ARM embedded toolchain."
    echo "On Ubuntu/Debian: sudo apt-get install gcc-arm-none-eabi"
    exit 1
fi

make clean

make release

mkdir -p "${INSTALL_PREFIX}/lib/arm-none-eabi"

cp build/libh6x_dynamic_packet_handler.a "${INSTALL_PREFIX}/lib/arm-none-eabi/"

echo "ARM Cortex-M build completed successfully!"
echo "Library installed to: ${INSTALL_PREFIX}/lib/arm-none-eabi/"