include(Arduino)

enable_arduino_toolchain()

set(sources main.cpp)

add_arduino_library(fk-core-tests "${sources}")

fk_configure_core(fk-core-tests)

add_arduino_firmware(fk-core-tests)
