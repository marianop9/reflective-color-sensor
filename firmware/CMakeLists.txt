cmake_minimum_required(VERSION 3.13)
f
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if (NOT BUILD_TESTING)
    # include pico-sdk from submodule (initialized in ./src)
    set(PICO_BOARD pico)
    include("${CMAKE_SOURCE_DIR}/external/pico-sdk/pico_sdk_init.cmake")
    # once pico_sdk_init.cmake is included, we're cross-compiling!
endif()

project(color-sensor C CXX ASM)
set(CMAKE_C_STANDARD 11)

option(BUILD_TESTING "Build tests" OFF)
include(CTest)

if (BUILD_TESTING)
    message(STATUS "=== Testing Configuration ===")
    enable_testing()
    add_subdirectory(tests)
endif()

add_subdirectory(src)