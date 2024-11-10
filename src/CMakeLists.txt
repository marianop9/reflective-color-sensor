add_subdirectory(lib/lcd)
add_subdirectory(lib/web_server)

add_executable(sensor main.c filter.c)

target_link_libraries(sensor
    pico_stdlib
    hardware_adc
    hardware_dma
    pico_multicore
    cs_web_server
    # mylib-lcd
)

pico_enable_stdio_usb(sensor 1)
pico_enable_stdio_uart(sensor 0)

pico_add_extra_outputs(sensor)
