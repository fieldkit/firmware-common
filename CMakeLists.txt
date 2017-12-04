project(ARDUINO_PROJECTS)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../arduino-cmake-minimal/cmake/")

include(ArduinoMinimal)

# TODO: Make creating internal libraries easier?
set(COMMON_SRCS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMMON_SOURCE_FILES
  ${COMMON_SRCS_DIR}/debug.cpp
  ${COMMON_SRCS_DIR}/i2c.cpp
  ${COMMON_SRCS_DIR}/active_object.cpp
  ${COMMON_SRCS_DIR}/utils.cpp
  ${COMMON_SRCS_DIR}/protobuf.cpp
  ${COMMON_SRCS_DIR}/module.cpp
  ${COMMON_SRCS_DIR}/module_messages.cpp
  ${COMMON_SRCS_DIR}/module_controller.cpp
  ${COMMON_SRCS_DIR}/module_servicer.cpp
  ${COMMON_SRCS_DIR}/pool.cpp
  ${COMMON_SRCS_DIR}/attached_devices.cpp
  ${COMMON_SRCS_DIR}/two_wire_task.cpp
  ${COMMON_SRCS_DIR}/app_messages.cpp
  ${COMMON_SRCS_DIR}/app_servicer.cpp
  ${COMMON_SRCS_DIR}/message_buffer.cpp
  ${COMMON_SRCS_DIR}/wifi.cpp
  ${COMMON_SRCS_DIR}/core_state.cpp
)

include_directories(${COMMON_SRCS_DIR})

add_subdirectory(module)
add_subdirectory(core)
