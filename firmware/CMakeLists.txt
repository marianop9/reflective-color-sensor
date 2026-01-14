cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# include pico-sdk from submodule (initialized in ./src)
set(PICO_BOARD pico)
include("${CMAKE_SOURCE_DIR}/external/pico-sdk/pico_sdk_init.cmake")

project(color-sensor C CXX ASM)
set(CMAKE_C_STANDARD 11)

add_subdirectory(src)