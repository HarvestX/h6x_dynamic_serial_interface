cmake_minimum_required(VERSION 3.16)
project(xv3heye_pc_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSERIAL REQUIRED libserial)

# Qt5 components for GUI application
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

include_directories(
    include
    ../h6x_dynamic_packet_handler/include
)

add_library(h6x_dynamic_packet_handler STATIC
    ../h6x_dynamic_packet_handler/src/h6x_dynamic_packet_crc8.c
    ../h6x_dynamic_packet_handler/src/h6x_dynamic_packet_handler.c
    ../h6x_dynamic_packet_handler/src/h6x_dynamic_packet_definitions_base.c
)

# == Qt5 GUI Application ===
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTORCC ON)


# == Packet Calculator GUI Application ===
add_executable(packet_calc_gui
    src/packet_calc_gui.cpp
    src/serial_interface.cpp
    src/packet_calc_gui.ui
)

target_link_libraries(packet_calc_gui
    Qt5::Core
    Qt5::Widgets
    ${LIBSERIAL_LIBRARIES}
    h6x_dynamic_packet_handler
)
target_compile_options(packet_calc_gui PRIVATE ${LIBSERIAL_CFLAGS_OTHER})