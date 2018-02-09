set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES
  Wire
  SD
  SPI
  ../module-protocol
)

set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/flash_with_bootloader_and_noinit.ld")

arduino(fk-core "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options( arduino_zero_fkfs PUBLIC -finstrument-functions)
target_compile_options(fk-core PUBLIC -Wall -Werror -finstrument-functions)
