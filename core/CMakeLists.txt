include(Arduino)

enable_arduino_toolchain()

find_package(FkCore)

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

set(sources main.cpp)

add_arduino_library(fk-core "${sources}")

fk_configure_core(fk-core)

fk_core_apply_configuration(fk-core)

fk_write_profile(fk-core)

add_arduino_firmware(fk-core)

option(FK_PROFILE_STANDARD "Standard Profile" ON)
if(FK_PROFILE_STANDARD)
  add_arduino_library(fk-core-standard "${sources}")

  fk_configure_core(fk-core-standard)

  target_compile_options(firmware-common-fk-core-standard PUBLIC -DFK_PROFILE_STANDARD)
  target_compile_options(firmware-common-fk-core-standard PUBLIC -DFK_CORE)
  target_compile_options(firmware-common-fk-core-standard PUBLIC -DFK_CORE_GENERATION_2)
  target_compile_options(firmware-common-fk-core-standard PUBLIC -DFK_DEBUG_UART_FALLBACK)
  target_compile_options(firmware-common-fk-core-standard PUBLIC -DFK_ENABLE_FLASH)
  target_compile_options(firmware-common-fk-core-standard PUBLIC -DFK_DISABLE_RADIO)
  target_compile_options(firmware-common-fk-core-standard PUBLIC -DFK_RTC_PCF8523)
  target_compile_options(firmware-common-fk-core-standard PUBLIC -DFK_HARDWARE_SERIAL2_ENABLE)

  fk_write_profile(fk-core-standard)

  add_arduino_firmware(fk-core-standard)
endif(FK_PROFILE_STANDARD)

option(FK_PROFILE_AMAZON "Amazon Profile" ON)
if(FK_PROFILE_AMAZON)
  add_arduino_library(fk-core-amazon "${sources}")

  fk_configure_core(fk-core-amazon)

  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_PROFILE_AMAZON)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_CORE)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_CORE_GENERATION_2)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_CORE_REQUIRE_MODULES)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_WIFI_STARTUP_ONLY)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_DEBUG_UART_FALLBACK)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_ENABLE_FLASH)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_DISABLE_RADIO)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_RTC_PCF8523)
  target_compile_options(firmware-common-fk-core-amazon PUBLIC -DFK_HARDWARE_SERIAL2_ENABLE)

  fk_write_profile(fk-core-amazon)

  add_arduino_firmware(fk-core-amazon)
endif(FK_PROFILE_AMAZON)

option(FK_DEBUG_INSTRUMENTATION "Enable function instrumentation." OFF)
if(FK_DEBUG_INSTRUMENTATION)
  target_compile_options(fk-core PUBLIC -Wall -Werror -finstrument-functions)
endif(FK_DEBUG_INSTRUMENTATION)
