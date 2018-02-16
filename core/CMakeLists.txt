set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES
  Wire
  SD
  SPI
  ../module-protocol
)

set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/flash_with_bootloader_and_noinit.ld")

add_definitions(-DFK_CORE)

option(DEBUG_UART_FALLBACK "On SerialUSB failure use Serial5" OFF)
if(DEBUG_UART_FALLBACK)
  add_definitions(-DDEBUG_UART_FALLBACK)
endif(DEBUG_UART_FALLBACK)

option(DEBUG_DUMP_OBJECT_SIZES "Dump object sizes at startup" OFF)
if(DEBUG_UART_FALLBACK)
  add_definitions(-DDEBUG_DUMP_OBJECT_SIZES)
endif(DEBUG_DUMP_OBJECT_SIZES)

option(DEBUG_MTB_ENABLE "Enable MicroTrace Buffer" OFF)
if(DEBUG_MTB_ENABLE)
  add_definitions(-DDEBUG_MTB_ENABLE)
endif(DEBUG_MTB_ENABLE)

arduino(fk-core "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(arduino_zero_Wire PUBLIC -finstrument-functions)
target_compile_options(arduino_zero_fkfs PUBLIC -finstrument-functions)
target_compile_options(arduino_zero_WiFi101 PUBLIC -finstrument-functions)
target_compile_options(fk-core PUBLIC -Wall -Werror -finstrument-functions)
