set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES Wire SD SPI ../module-protocol)

include(ArduinoMinimal)

arduino(fk-tests "${SOURCE_FILES}" "${LIBRARIES}")
