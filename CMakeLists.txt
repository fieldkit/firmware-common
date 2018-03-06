cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(FIRMWARE_COMMON)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../arduino-cmake-minimal/cmake/")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(ArduinoMinimal)

# TODO: Make creating internal libraries easier?
set(COMMON_SRCS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMMON_SOURCE_FILES
  ${COMMON_SRCS_DIR}/debug.cpp
  ${COMMON_SRCS_DIR}/hardware.cpp
  ${COMMON_SRCS_DIR}/two_wire.cpp
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
  ${COMMON_SRCS_DIR}/wifi_connections.cpp
  ${COMMON_SRCS_DIR}/core_state.cpp
  ${COMMON_SRCS_DIR}/watchdog.cpp
  ${COMMON_SRCS_DIR}/live_data.cpp
  ${COMMON_SRCS_DIR}/scheduler.cpp
  ${COMMON_SRCS_DIR}/simple_ntp.cpp
  ${COMMON_SRCS_DIR}/http_post.cpp
  ${COMMON_SRCS_DIR}/fkfs_replies.cpp
  ${COMMON_SRCS_DIR}/fkfs_data.cpp
  ${COMMON_SRCS_DIR}/fkfs_tasks.cpp
  ${COMMON_SRCS_DIR}/core_module.cpp
  ${COMMON_SRCS_DIR}/discovery.cpp
  ${COMMON_SRCS_DIR}/utils.cpp
  ${COMMON_SRCS_DIR}/json_message_builder.cpp
  ${COMMON_SRCS_DIR}/leds.cpp
  ${COMMON_SRCS_DIR}/power_management.cpp
  ${COMMON_SRCS_DIR}/rtc.cpp
  ${COMMON_SRCS_DIR}/device_id.cpp
  ${COMMON_SRCS_DIR}/transmissions.cpp
  ${COMMON_SRCS_DIR}/transmit_readings.cpp
  ${COMMON_SRCS_DIR}/file_system.cpp
  ${COMMON_SRCS_DIR}/restart_wizard.cpp
  ${COMMON_SRCS_DIR}/data_messages.cpp
  ${COMMON_SRCS_DIR}/gps.cpp
  ${COMMON_SRCS_DIR}/download_file_task.cpp
  ${COMMON_SRCS_DIR}/gather_readings.cpp
  ${COMMON_SRCS_DIR}/flash_storage.cpp
  ${COMMON_SRCS_DIR}/app_module_query_task.cpp
)

include_directories(${COMMON_SRCS_DIR})

add_subdirectory(module)
add_subdirectory(core)
