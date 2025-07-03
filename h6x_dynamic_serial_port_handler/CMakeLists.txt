cmake_minimum_required(VERSION 3.16)
project(h6x_dynamic_serial_port_handler)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

find_package(PkgConfig REQUIRED)
pkg_check_modules(libserial REQUIRED libserial)

ament_auto_add_library(${PROJECT_NAME} SHARED
  src/serial_port_handler.cpp
  src/libserial_helper.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${libserial_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  ${libserial_LIBRARIES}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()