set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp example_module.cpp)
set(LIBRARIES SPI Wire)

include(ArduinoMinimal)

arduino(fk-module "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(fk-module PUBLIC -Wall -Werror)

add_definitions(-DPHYLUM_ENABLE_SERIAL_FLASH -DPHYLUM_ENABLE_SD)
target_compile_options(arduino_zero_phylum PUBLIC -DPHYLUM_ENABLE_SERIAL_FLASH -DPHYLUM_ENABLE_SD)
