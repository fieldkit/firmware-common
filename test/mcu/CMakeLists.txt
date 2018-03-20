set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES
  Wire
  SD
  SPI
  ../module-protocol
)

include(ArduinoMinimal)

arduino(fk-firmware-test "${SOURCE_FILES}" "${LIBRARIES}")
