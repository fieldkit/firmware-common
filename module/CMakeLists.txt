include(Arduino)

enable_arduino_toolchain()

set(firmware-common_PATH ../src)

find_package(FkModule)

set(sources main.cpp example_module.cpp)

add_arduino_library(fk-testing-module-standard "${sources}")

fk_configure_module(fk-testing-module-standard)

add_arduino_firmware(fk-testing-module-standard)
