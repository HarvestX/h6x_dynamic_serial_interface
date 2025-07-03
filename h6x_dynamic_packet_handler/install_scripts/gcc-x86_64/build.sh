#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_PREFIX="${1:-/usr/local}"

echo "Building h6x_dynamic_packet_handler for native gcc..."

if ! command -v gcc &> /dev/null; then
    echo "Error: gcc not found. Please install build-essential."
    echo "On Ubuntu/Debian: sudo apt-get install build-essential"
    exit 1
fi

make clean

make release

mkdir -p "${INSTALL_PREFIX}/lib/x86_64-linux-gnu"

cp build/libh6x_dynamic_packet_handler.so.1.0.0 "${INSTALL_PREFIX}/lib/x86_64-linux-gnu/"
cp build/libh6x_dynamic_packet_handler.so.1 "${INSTALL_PREFIX}/lib/x86_64-linux-gnu/"
cp build/libh6x_dynamic_packet_handler.so "${INSTALL_PREFIX}/lib/x86_64-linux-gnu/"

echo "x86_64 Linux build completed successfully!"
echo "Library installed to: ${INSTALL_PREFIX}/lib/x86_64-linux-gnu/"