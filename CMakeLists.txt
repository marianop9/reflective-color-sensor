cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
set(PICO_BOARD pico_w)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(pico-sdk/pico_sdk_init.cmake)

project(color-sensor C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(src)
