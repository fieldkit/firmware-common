include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)

file(GLOB sources *.cpp
  ../../../src/debug.cpp
  ../../../src/pool.cpp
  ../../../src/http_response_parser.cpp
)

add_executable(testcommon "${sources}")

target_include_directories(testcommon PRIVATE "${arduino-logging_PATH}/src")

target_link_libraries(testcommon libgtest libgmock)

set_target_properties(testcommon PROPERTIES C_STANDARD 11)
set_target_properties(testcommon PROPERTIES CXX_STANDARD 11)

add_test(NAME testcommon COMMAND testcommon)
