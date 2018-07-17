set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES
  Wire
  SD
  SPI
  ../module-protocol
)

set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/flash_with_bootloader_and_noinit.ld")

option(FK_DEBUG_UART_FALLBACK "On SerialUSB failure use Serial5" ON)
if(DEBUG_UART_FALLBACK)
  add_definitions(-DFK_DEBUG_UART_FALLBACK)
endif(FK_DEBUG_UART_FALLBACK)

option(FK_DEBUG_UART_REQUIRE_CONSOLE "Wait forever for a console connection" OFF)
if(FK_DEBUG_UART_REQUIRE_CONSOLE)
  add_definitions(-DFK_DEBUG_UART_REQUIRE_CONSOLE)
endif(FK_DEBUG_UART_REQUIRE_CONSOLE)

option(FK_DEBUG_MTB_ENABLE "Enable MicroTrace Buffer" OFF)
if(FK_DEBUG_MTB_ENABLE)
  add_definitions(-DFK_DEBUG_MTB_ENABLE)
endif(FK_DEBUG_MTB_ENABLE)

option(FK_API_BASE "Use given API_BASE")
if(FK_API_BASE)
  add_definitions(-DFK_API_BASE="${API_BASE}")
endif(FK_API_BASE)

option(FK_CORE_GENERATION_2 "Enable Generation 2 Core hardware profile. (see harware.h)")
if(FK_CORE_GENERATION_2)
  add_definitions(-DFK_CORE)
  add_definitions(-DFK_CORE_GENERATION_2)
else()
  add_definitions(-DFK_CORE)
  add_definitions(-DFK_CORE_GENERATION_1)
endif(FK_CORE_GENERATION_2)

option(FK_ENABLE_FLASH "Enable/disable flash memory." ON)
if(FK_ENABLE_FLASH)
  add_definitions(-DFK_ENABLE_FLASH)
else()
  add_definitions(-DFK_DISABLE_FLASH)
endif(FK_ENABLE_FLASH)

option(FK_ENABLE_RADIO "Enable/disable LoRa radio." ON)
if(FK_ENABLE_RADIO)
  add_definitions(-DFK_DISABLE_RADIO)
else()
  add_definitions(-DFK_ENABLE_RADIO)
endif(FK_ENABLE_RADIO)

option(FK_WIFI_ALWAYS_ON "If ON never turn off Wifi" OFF)
if(FK_WIFI_ALWAYS_ON)
  add_definitions(-DFK_WIFI_ALWAYS_ON)
endif(FK_WIFI_ALWAYS_ON)

option(FK_WIFI_STARTUP_ONLY "Only run Wifi on startup" OFF)
if(FK_WIFI_STARTUP_ONLY)
  add_definitions(-DFK_WIFI_STARTUP_ONLY)
endif(FK_WIFI_STARTUP_ONLY)

option(FK_PROFILE_AMAZON "Amazon Profile")
if(FK_PROFILE_AMAZON)
 add_definitions(-DFK_PROFILE_AMAZON)
endif(FK_PROFILE_AMAZON)

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
