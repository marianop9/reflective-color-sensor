add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/ascii-comm-proto)

# avoid including pico-sdk, freeRTOS, etc for host builds (tests) 
if (CMAKE_CROSSCOMPILING)
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

    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/usbproto)

    # main executable
    add_executable(firmware
        main.c
        hooks.c
    )
    
    target_link_libraries(firmware
        pico_stdlib
        # hardware_adc
        # hardware_dma
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4
        usbproto
        ascii-comm-proto
    )
    
    pico_enable_stdio_usb(firmware 1)
    pico_enable_stdio_uart(firmware 0)
    # generate uf2
    pico_add_extra_outputs(firmware)
    
endif()
