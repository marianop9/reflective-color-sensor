add_subdirectory(lib/lcd)

add_executable(sensor main.c)

target_link_libraries(sensor
    pico_stdlib
    # pico_cyw43_arch_none
    hardware_adc
    hardware_dma
    mylib-lcd
    )

pico_enable_stdio_usb(sensor 1)
pico_enable_stdio_uart(sensor 0)

pico_add_extra_outputs(sensor)
