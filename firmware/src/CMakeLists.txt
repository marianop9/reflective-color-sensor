add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/ascii-comm-proto)

# avoid including pico-sdk, freeRTOS, etc for host builds (tests) 
if (CMAKE_CROSSCOMPILING)
    set(TINYUSB_OPT_OS OPT_OS_FREERTOS)
    # init pico-sdk
    pico_sdk_init()
    # include FreeRTOS Kernel
    ## the RP2040 FreeRTOS port expects the (deprecated) FREERTOS_CONFIG_FILE_DIRECTORY variable
    set(FREERTOS_CONFIG_FILE_DIRECTORY 
        "${CMAKE_CURRENT_LIST_DIR}"
        CACHE PATH
        "Directory containing FreeRTOSConfig.h"
    )
    add_subdirectory(
        ${CMAKE_SOURCE_DIR}/external/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040 
        FreeRTOS-Kernel
    )

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/bsp)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/drivers)

    # main executable
    add_executable(firmware
        main.c
        hooks.c
        usb_descriptors.c
    )
    
    target_link_libraries(firmware
        pico_stdlib
        tinyusb_device
        tinyusb_board
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4
        ascii-comm-proto
        driver_adc
        driver_cd4066_resistor_picker
        driver_ws2812
        bsp
    )
    
    pico_enable_stdio_usb(firmware 0)
    pico_enable_stdio_uart(firmware 0)
    # generate uf2
    pico_add_extra_outputs(firmware)
    
endif()
