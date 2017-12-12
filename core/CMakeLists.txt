set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES
  Wire
  SD
  SPI
  ../module-protocol
)

arduino(fk-core "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(fk-core PUBLIC -Wall -Werror)
