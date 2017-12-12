set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp)

set(LIBRARIES
  SPI
  Wire
  ../module-protocol
)

set(EXTRA_CXX_FLAGS_PROJECT "-Wall -Werror")

arduino(fk-module "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(fk-module PUBLIC -Wall -Werror)
