# h6x_dynamic_packet_handlerTargets.cmake
# CMake targets file for h6x_dynamic_packet_handler

# Create imported target h6x_dynamic_packet_handler::h6x_dynamic_packet_handler
add_library(h6x_dynamic_packet_handler::h6x_dynamic_packet_handler SHARED IMPORTED)

# Set target properties
set_target_properties(h6x_dynamic_packet_handler::h6x_dynamic_packet_handler PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../include"
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../../../lib/x86_64-linux-gnu/libh6x_dynamic_packet_handler.so"
)