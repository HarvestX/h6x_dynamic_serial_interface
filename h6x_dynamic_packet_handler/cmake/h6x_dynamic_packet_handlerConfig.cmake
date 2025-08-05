# h6x_dynamic_packet_handlerConfig.cmake
# CMake configuration file for h6x_dynamic_packet_handler

include(CMakeFindDependencyMacro)

# Define the library target
if(NOT TARGET h6x_dynamic_packet_handler::h6x_dynamic_packet_handler)
    include("${CMAKE_CURRENT_LIST_DIR}/h6x_dynamic_packet_handlerTargets.cmake")
endif()

# Set variables for compatibility
set(h6x_dynamic_packet_handler_FOUND TRUE)
set(h6x_dynamic_packet_handler_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../../include")
set(h6x_dynamic_packet_handler_LIBRARIES h6x_dynamic_packet_handler::h6x_dynamic_packet_handler)