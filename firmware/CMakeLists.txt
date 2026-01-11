cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
set(PICO_BOARD pico_w)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# FreeRTOS paths
set(FREERTOS_SRC_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/FreeRTOS-Kernel")
set(FREERTOS_CFG_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

message(STATUS "Using FreeRTOS from: ${FREERTOS_SRC_DIRECTORY}")
message(STATUS "Using FreeRTOSConfig from: ${FREERTOS_CFG_DIRECTORY}")
message(STATUS "##################################################################################################################")

include(external/pico-sdk/pico_sdk_init.cmake)
project(color-sensor C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_library(FreeRTOS STATIC
    ${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/RP2040/port.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_4.c
    ${FREERTOS_SRC_DIRECTORY}/croutine.c
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
)

target_include_directories(FreeRTOS PUBLIC
    ${CMAKE_SOURCE_DIR}/firmware
    ${CMAKE_CURRENT_SOURCE_DIR}/external/pico-sdk/src/common/pico_base_headers/include
    ${FREERTOS_CFG_DIRECTORY}
    ${FREERTOS_SRC_DIRECTORY}/include
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/Posix
    ${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/RP2040/include
)

add_subdirectory(src)
