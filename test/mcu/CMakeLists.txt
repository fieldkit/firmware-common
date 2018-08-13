set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES Wire SD SPI)

include(ArduinoMinimal)

arduino(fk-tests "${SOURCE_FILES}" "${LIBRARIES}")

add_definitions(-DPHYLUM_ENABLE_SERIAL_FLASH -DPHYLUM_ENABLE_SD)
target_compile_options(arduino_zero_phylum PUBLIC -DPHYLUM_ENABLE_SERIAL_FLASH -DPHYLUM_ENABLE_SD)
