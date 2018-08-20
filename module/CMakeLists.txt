set(SOURCE_FILES ${COMMON_SOURCE_FILES} main.cpp example_module.cpp)
set(LIBRARIES SPI Wire)

add_definitions("-DFIRMWARE_GIT_HASH=\"${GIT_COMMIT_HASH}\"")
add_definitions("-DFIRMWARE_GIT_BRANCH=\"${GIT_BRANCH}\"")
add_definitions("-DFIRMWARE_BUILD=\"$ENV{BUILD_TAG}\"")

include(ArduinoMinimal)

arduino(fk-testing-module-standard "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(fk-testing-module-standard PUBLIC -Wall -Werror)

add_definitions(-DPHYLUM_ENABLE_SERIAL_FLASH -DPHYLUM_ENABLE_SD)
target_compile_options(arduino_zero_phylum PUBLIC -DPHYLUM_ENABLE_SERIAL_FLASH -DPHYLUM_ENABLE_SD)
