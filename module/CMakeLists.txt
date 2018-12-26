include(Arduino)

enable_arduino_toolchain()

set(firmware-common_PATH ../)

find_package(FkModule)

set(sources main.cpp example_module.cpp)

add_arduino_library(fk-example-module-standard "${sources}")

fk_configure_module(fk-example-module-standard)

add_arduino_firmware(fk-example-module-standard)
