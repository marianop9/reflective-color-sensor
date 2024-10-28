add_executable(sensor main.c)

add_subdirectory(lib/lcd)

target_link_libraries(sensor
    pico_stdlib
    pico_cyw43_arch_none
    mylib-lcd
    )

pico_enable_stdio_usb(sensor 1)
pico_enable_stdio_uart(sensor 0)

pico_add_extra_outputs(sensor)
