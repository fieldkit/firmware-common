set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES Wire SD SPI)

include(ArduinoMinimal)

arduino(fk-tests "${SOURCE_FILES}" "${LIBRARIES}")
