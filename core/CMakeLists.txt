set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)
set(LIBRARIES Wire SD SPI ../module-protocol)

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

option(FK_BOOTLOADER_LARGE "Require module, restarting if none are found. " ON)
if(FK_BOOTLOADER_LARGE)
  set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_bootloader_large.ld")
  add_definitions(-DFK_BOOTLOADER_LARGE)
endif(FK_BOOTLOADER_LARGE)

arduino(fk-core "${SOURCE_FILES}" "${LIBRARIES}")

option(FK_DEBUG_UART_FALLBACK "On SerialUSB failure use Serial5" ON)
if(FK_DEBUG_UART_FALLBACK)
  target_compile_options(fk-core PUBLIC -DFK_DEBUG_UART_FALLBACK)
endif(FK_DEBUG_UART_FALLBACK)

option(FK_DEBUG_UART_REQUIRE_CONSOLE "Wait forever for a console connection" OFF)
if(FK_DEBUG_UART_REQUIRE_CONSOLE)
  target_compile_options(fk-core PUBLIC -DFK_DEBUG_UART_REQUIRE_CONSOLE)
endif(FK_DEBUG_UART_REQUIRE_CONSOLE)

option(FK_DEBUG_MTB_ENABLE "Enable MicroTrace Buffer" OFF)
if(FK_DEBUG_MTB_ENABLE)
  target_compile_options(fk-core PUBLIC -DFK_DEBUG_MTB_ENABLE)
endif(FK_DEBUG_MTB_ENABLE)

option(FK_API_BASE "Use given API_BASE")
if(FK_API_BASE)
  target_compile_options(fk-core PUBLIC -DFK_API_BASE="${API_BASE}")
endif(FK_API_BASE)

option(FK_CORE_GENERATION_2 "Enable Generation 2 Core hardware profile. (see harware.h)" ON)
if(FK_CORE_GENERATION_2)
  target_compile_options(fk-core PUBLIC -DFK_CORE)
  target_compile_options(fk-core PUBLIC -DFK_CORE_GENERATION_2)
else()
  target_compile_options(fk-core PUBLIC -DFK_CORE)
  target_compile_options(fk-core PUBLIC -DFK_CORE_GENERATION_1)
endif(FK_CORE_GENERATION_2)

option(FK_ENABLE_FLASH "Enable/disable flash memory." ON)
if(FK_ENABLE_FLASH)
  target_compile_options(fk-core PUBLIC -DFK_ENABLE_FLASH)
else()
  target_compile_options(fk-core PUBLIC -DFK_DISABLE_FLASH)
endif(FK_ENABLE_FLASH)

option(FK_ENABLE_LOW_POWER_SLEEP "Enable low power sleep." ON)
if(FK_ENABLE_LOW_POWER_SLEEP)
  target_compile_options(fk-core PUBLIC -DFK_ENABLE_LOW_POWER_SLEEP)
else()
  target_compile_options(fk-core PUBLIC -DFK_DISABLE_LOW_POWER_SLEEP)
endif(FK_ENABLE_LOW_POWER_SLEEP)

option(FK_ENABLE_DEEP_SLEEP "Enable deep sleep." ON)
if(FK_ENABLE_DEEP_SLEEP)
  target_compile_options(fk-core PUBLIC -DFK_ENABLE_DEEP_SLEEP)
else()
  target_compile_options(fk-core PUBLIC -DFK_DISABLE_DEEP_SLEEP)
endif(FK_ENABLE_DEEP_SLEEP)

option(FK_ENABLE_RADIO "Enable/disable LoRa radio." OFF)
if(FK_ENABLE_RADIO)
  target_compile_options(fk-core PUBLIC -DFK_DISABLE_RADIO)
else()
  target_compile_options(fk-core PUBLIC -DFK_ENABLE_RADIO)
endif(FK_ENABLE_RADIO)

option(FK_WIFI_ALWAYS_ON "If ON never turn off Wifi" OFF)
if(FK_WIFI_ALWAYS_ON)
  target_compile_options(fk-core PUBLIC -DFK_WIFI_ALWAYS_ON)
endif(FK_WIFI_ALWAYS_ON)

option(FK_WIFI_STARTUP_ONLY "Only run Wifi on startup" OFF)
if(FK_WIFI_STARTUP_ONLY)
  target_compile_options(fk-core PUBLIC -DFK_WIFI_STARTUP_ONLY)
endif(FK_WIFI_STARTUP_ONLY)

option(FK_CORE_REQUIRE_MODULES "Require module, restarting if none are found. " OFF)
if(FK_CORE_REQUIRE_MODULES)
  target_compile_options(fk-core PUBLIC -DFK_CORE_REQUIRE_MODULES)
endif(FK_CORE_REQUIRE_MODULES)

option(FK_PROFILE_STANDARD "Standard Profile" ON)
if(FK_PROFILE_STANDARD)
  arduino(fk-core-standard "${SOURCE_FILES}" "${LIBRARIES}")
  target_compile_options(fk-core-standard PUBLIC -DFK_PROFILE_STANDARD)
  target_compile_options(fk-core-standard PUBLIC -DFK_CORE)
  target_compile_options(fk-core-standard PUBLIC -DFK_CORE_GENERATION_2)
  target_compile_options(fk-core-standard PUBLIC -DFK_DEBUG_UART_FALLBACK)
  target_compile_options(fk-core-standard PUBLIC -DFK_ENABLE_FLASH)
  target_compile_options(fk-core-standard PUBLIC -DFK_DISABLE_RADIO)
endif(FK_PROFILE_STANDARD)

option(FK_PROFILE_AMAZON "Amazon Profile" ON)
if(FK_PROFILE_AMAZON)
  arduino(fk-core-amazon "${SOURCE_FILES}" "${LIBRARIES}")
  target_compile_options(fk-core-amazon PUBLIC -DFK_PROFILE_AMAZON)
  target_compile_options(fk-core-amazon PUBLIC -DFK_CORE)
  target_compile_options(fk-core-amazon PUBLIC -DFK_CORE_GENERATION_2)
  target_compile_options(fk-core-amazon PUBLIC -DFK_CORE_REQUIRE_MODULES)
  target_compile_options(fk-core-amazon PUBLIC -DFK_WIFI_STARTUP_ONLY)
  target_compile_options(fk-core-amazon PUBLIC -DFK_DEBUG_UART_FALLBACK)
  target_compile_options(fk-core-amazon PUBLIC -DFK_ENABLE_FLASH)
  target_compile_options(fk-core-amazon PUBLIC -DFK_DISABLE_RADIO)
endif(FK_PROFILE_AMAZON)

option(FK_INSTRUMENTATION "Enable function instrumentation.")
if(FK_INSTRUMENTATION)
  target_compile_options(arduino_zero_Wire PUBLIC -finstrument-functions)
  target_compile_options(arduino_zero_phylum PUBLIC -finstrument-functions)
  target_compile_options(arduino_zero_WiFi101 PUBLIC -finstrument-functions)
  target_compile_options(fk-core PUBLIC -Wall -Werror -finstrument-functions)
endif(FK_INSTRUMENTATION)
