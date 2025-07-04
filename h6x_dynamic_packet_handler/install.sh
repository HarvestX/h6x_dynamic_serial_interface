#!/bin/bash

# h6x_dynamic_packet_handler multi-platform installation/uninstallation script
# This script installs or uninstalls the h6x_dynamic_packet_handler library to/from /usr/local/ for multiple platforms

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_PREFIX="/usr/local"
UNINSTALL_MODE=false

PLATFORMS=("arm-none-eabi" "arm-none-eabi-stm32f4" "gcc-x86_64")

if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root or with sudo"
    exit 1
fi

BUILD_PLATFORMS=()
SHOW_HELP=false

while [[ $# -gt 0 ]]; do
    case $1 in
        uninstall)
            UNINSTALL_MODE=true
            shift
            ;;
        --platforms)
            shift
            while [[ $# -gt 0 && $1 != --* ]]; do
                BUILD_PLATFORMS+=("$1")
                shift
            done
            ;;
        --help|-h)
            SHOW_HELP=true
            shift
            ;;
        --*)
            echo "Unknown option: $1"
            SHOW_HELP=true
            shift
            ;;
        *)
            if [[ -z "$INSTALL_PREFIX_SET" ]]; then
                INSTALL_PREFIX="$1"
                INSTALL_PREFIX_SET=true
            fi
            shift
            ;;
    esac
done

if [[ "$SHOW_HELP" == true ]]; then
    echo "Usage: $0 [INSTALL_PREFIX] [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  uninstall                            Uninstall the library"
    echo ""
    echo "Options:"
    echo "  --platforms PLATFORM1 PLATFORM2 ... Build only specified platforms (install only)"
    echo "  --help, -h                           Show this help message"
    echo ""
    echo "Available platforms:"
    for platform in "${PLATFORMS[@]}"; do
        echo "  - $platform"
    done
    echo ""
    echo "Examples:"
    echo "  $0 /usr/local                               Install all platforms"
    echo "  $0 /usr/local --platforms gcc-x86_64        Install only gcc-x86_64"
    echo "  $0 /usr/local --platforms arm-none-eabi-stm32f4 Install only STM32F4 with FPU"
    echo "  $0 /usr/local uninstall                     Uninstall from /usr/local"
    exit 0
fi

INCLUDE_DIR="${INSTALL_PREFIX}/include"
LIB_DIR="${INSTALL_PREFIX}/lib"
CMAKE_DIR="${INSTALL_PREFIX}/lib/cmake/h6x_dynamic_packet_handler"

if [[ ! -d "${INSTALL_PREFIX}" ]]; then
    echo "Error: INSTALL_PREFIX '${INSTALL_PREFIX}' is not a directory."
    echo "Please provide a valid directory path."
    exit 1
fi

uninstall_library() {
    echo "h6x_dynamic_packet_handler uninstall script"
    echo "Uninstalling from: ${INSTALL_PREFIX}"
    echo ""
    
    if [ -d "${INCLUDE_DIR}/h6x_dynamic_packet_handler" ]; then
        echo "Removing header files from ${INCLUDE_DIR}/h6x_dynamic_packet_handler..."
        rm -rf "${INCLUDE_DIR}/h6x_dynamic_packet_handler"
    fi
    
    echo "Removing library files..."
    rm -f "${LIB_DIR}"/*/libh6x_dynamic_packet_handler.*
    rm -f "${LIB_DIR}"/libh6x_dynamic_packet_handler.*
    
    for platform in "${PLATFORMS[@]}"; do
        case $platform in
            "arm-none-eabi")
                if [ -d "${LIB_DIR}/h6x_dynamic_packet_handler/arm-none-eabi" ]; then
                    echo "Removing arm-none-eabi library directory..."
                    rm -rf "${LIB_DIR}/h6x_dynamic_packet_handler/arm-none-eabi"
                fi
                ;;
            "arm-none-eabi-stm32f4")
                if [ -f "${LIB_DIR}/arm-none-eabi/libh6x_dynamic_packet_handler_stm32f4.a" ]; then
                    echo "Removing arm-none-eabi-stm32f4 library file..."
                    rm -f "${LIB_DIR}/arm-none-eabi/libh6x_dynamic_packet_handler_stm32f4.a"
                fi
                ;;
            "gcc-x86_64")
                if [ -d "${LIB_DIR}/x86_64-linux-gnu" ]; then
                    echo "Removing x86_64-linux-gnu library files..."
                    rm -f "${LIB_DIR}/x86_64-linux-gnu/libh6x_dynamic_packet_handler.*"
                fi
                ;;
        esac
    done
    
    if [ -d "${LIB_DIR}/h6x_dynamic_packet_handler" ]; then
        rmdir "${LIB_DIR}/h6x_dynamic_packet_handler" 2>/dev/null || true
    fi
    
    if [ -d "${CMAKE_DIR}" ]; then
        echo "Removing CMake config files from ${CMAKE_DIR}..."
        rm -rf "${CMAKE_DIR}"
    fi
    
    echo "Updating library cache..."
    ldconfig
    
    echo ""
    echo "Uninstallation completed successfully!"
    exit 0
}

if [[ "$UNINSTALL_MODE" == true ]]; then
    uninstall_library
fi

if [[ ${#BUILD_PLATFORMS[@]} -eq 0 ]]; then
    BUILD_PLATFORMS=("${PLATFORMS[@]}")
fi

echo "h6x_dynamic_packet_handler multi-platform installation script"
echo "Installing to: ${INSTALL_PREFIX}"
echo ""

mkdir -p "${INCLUDE_DIR}/h6x_dynamic_packet_handler"
mkdir -p "${LIB_DIR}"
mkdir -p "${CMAKE_DIR}"

echo "Installing header files..."
cp -r "${SCRIPT_DIR}/include/h6x_dynamic_packet_handler"/* "${INCLUDE_DIR}/h6x_dynamic_packet_handler/"

for platform in "${BUILD_PLATFORMS[@]}"; do
    echo ""
    echo "Building for platform: $platform"
    echo "================================"
    
    case $platform in
        "arm-none-eabi-stm32f4")
            if [[ ! -d "${SCRIPT_DIR}/install_scripts/arm-none-eabi" ]]; then
                echo "Warning: arm-none-eabi platform not found, skipping..."
                continue
            fi
            cd "${SCRIPT_DIR}/install_scripts/arm-none-eabi"
            ./build.sh "${INSTALL_PREFIX}" "stm32f4"
            ;;
        *)
            if [[ ! -d "${SCRIPT_DIR}/install_scripts/${platform}" ]]; then
                echo "Warning: Platform ${platform} not found, skipping..."
                continue
            fi
            cd "${SCRIPT_DIR}/install_scripts/${platform}"
            ./build.sh "${INSTALL_PREFIX}"
            ;;
    esac
done

echo "Installing CMake config files..."
cp -r "${SCRIPT_DIR}/cmake/"* "${CMAKE_DIR}/"

ldconfig


echo ""
echo "Installation completed successfully!"
echo "Headers installed to: ${INCLUDE_DIR}/h6x_dynamic_packet_handler/"
echo "Libraries installed to:"
for platform in "${BUILD_PLATFORMS[@]}"; do
    case $platform in
        "arm-none-eabi")
            echo "  - ${LIB_DIR}/arm-none-eabi/libh6x_dynamic_packet_handler.a (static)"
            ;;
        "arm-none-eabi-stm32f4")
            echo "  - ${LIB_DIR}/arm-none-eabi/libh6x_dynamic_packet_handler_stm32f4.a (static)"
            ;;
        "gcc-x86_64")
            echo "  - ${LIB_DIR}/x86_64-linux-gnu/libh6x_dynamic_packet_handler.so (shared)"
            ;;
    esac
done
