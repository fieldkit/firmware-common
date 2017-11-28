set(SOURCE_FILES
  test.cpp
)

set(LIBRARIES
  SPI
  Wire
  ../modules
)

arduino(test-module "${SOURCE_FILES}" "${LIBRARIES}")
