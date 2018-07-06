set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES
  Wire
  SD
  SPI
  ../module-protocol
)

set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/flash_with_bootloader_and_noinit.ld")

add_definitions(-DFK_CORE)

option(DEBUG_UART_FALLBACK "On SerialUSB failure use Serial5" ON)
if(DEBUG_UART_FALLBACK)
  add_definitions(-DDEBUG_UART_FALLBACK)
endif(DEBUG_UART_FALLBACK)

option(DEBUG_DUMP_OBJECT_SIZES "Dump object sizes at startup" OFF)
if(DEBUG_DUMP_OBJECT_SIZES)
  add_definitions(-DDEBUG_DUMP_OBJECT_SIZES)
endif(DEBUG_DUMP_OBJECT_SIZES)

option(DEBUG_MTB_ENABLE "Enable MicroTrace Buffer" OFF)
if(DEBUG_MTB_ENABLE)
  add_definitions(-DDEBUG_MTB_ENABLE)
endif(DEBUG_MTB_ENABLE)

option(API_BASE "Use given API_BASE")
if(API_BASE)
  add_definitions(-DFK_API_BASE="${API_BASE}")
endif(API_BASE)

option(NO_NETWORK_CONFIG "Disable preconfiguring network.")
if(NO_NETWORK_CONFIG)
  add_definitions(-DFK_NO_NETWORKS)
endif(NO_NETWORK_CONFIG)

option(DISABLE_FLASH "Disable preconfiguring network.")
if(DISABLE_FLASH)
  add_definitions(-DFK_DISABLE_FLASH)
endif(DISABLE_FLASH)

option(DISABLE_RADIO "Disable radio.")
if(DISABLE_RADIO)
else()
  add_definitions(-DFK_ENABLE_RADIO)
endif(DISABLE_RADIO)

option(WIFI_ALWAYS_ON "Never disable Wifi.")
if(WIFI_ALWAYS_ON)
  add_definitions(-DFK_WIFI_ALWAYS_ON)
endif(WIFI_ALWAYS_ON)

option(CORE_GENERATION_1 "Enable Generation 1 Core hardware profile. (see harware.h)")
if(CORE_GENERATION_1)
  add_definitions(-DFK_CORE_GENERATION_1)
else()
  add_definitions(-DFK_CORE_GENERATION_2)
endif(CORE_GENERATION_1)

option(CORE_GENERATION_2 "Enable Generation 2 Core hardware profile. (see harware.h)")
if(CORE_GENERATION_2)
  add_definitions(-DFK_CORE_GENERATION_2)
else()
  add_definitions(-DFK_CORE_GENERATION_1)
endif(CORE_GENERATION_2)

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%H
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions("-DFIRMWARE_GIT_HASH=\"${GIT_COMMIT_HASH}\"")
add_definitions("-DFIRMWARE_GIT_BRANCH=\"${GIT_BRANCH}\"")
add_definitions("-DFIRMWARE_BUILD=\"$ENV{BUILD_TAG}\"")

include(ArduinoMinimal)

arduino(fk-core "${SOURCE_FILES}" "${LIBRARIES}")

option(INSTRUMENTATION "Enable function instrumentation.")
if(INSTRUMENTATION)
  target_compile_options(arduino_zero_Wire PUBLIC -finstrument-functions)
  target_compile_options(arduino_zero_phylum PUBLIC -finstrument-functions)
  target_compile_options(arduino_zero_WiFi101 PUBLIC -finstrument-functions)
  target_compile_options(fk-core PUBLIC -Wall -Werror -finstrument-functions)
endif(INSTRUMENTATION)
