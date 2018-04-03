set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp example_module.cpp)

set(LIBRARIES
  SPI
  Wire
  ../module-protocol
)

include(ArduinoMinimal)

arduino(fk-module "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(arduino_zero_Wire PUBLIC -finstrument-functions)
target_compile_options(fk-module PUBLIC -Wall -Werror)
