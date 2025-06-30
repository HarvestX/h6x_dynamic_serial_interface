cmake_minimum_required(VERSION 3.16)
project(h6x_dynamic_packet_handler)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# ===================================================================
set(TARGET h6x_dynamic_packet_handler)
ament_auto_add_library(${TARGET} SHARED
  ./src/h6x_dynamic_packet_crc8.c
  ./src/h6x_dynamic_packet_definitions_base.c
  ./src/h6x_dynamic_packet_handler.c
)

# ===================================================================
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()