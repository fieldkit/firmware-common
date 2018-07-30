set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp example_module.cpp)
set(LIBRARIES SPI Wire ../module-protocol)

include(ArduinoMinimal)

arduino(fk-module-test "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(fk-module-test PUBLIC -Wall -Werror)
