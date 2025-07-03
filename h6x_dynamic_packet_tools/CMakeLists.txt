cmake_minimum_required(VERSION 3.16)
project(h6x_dynamic_packet_tools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSERIAL REQUIRED libserial)

# Qt5 components for GUI application
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

ament_auto_add_executable(packet_calc_gui
    src/packet_calc_gui.cpp
    src/packet_calc_gui.ui
)

target_link_libraries(packet_calc_gui
    Qt5::Core
    Qt5::Widgets
    ${LIBSERIAL_LIBRARIES}
)
target_compile_options(packet_calc_gui PRIVATE ${LIBSERIAL_CFLAGS_OTHER})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()