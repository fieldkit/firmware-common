set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES
  Wire
  SD
  SPI
  ../module-protocol
)

set(EXTRA_CXX_FLAGS_PROJECT "-Wall -Werror")

arduino(fk-core "${SOURCE_FILES}" "${LIBRARIES}")
