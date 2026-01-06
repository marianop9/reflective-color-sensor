message(STATUS "Testing Configuration")

add_library(unity STATIC unity/unity.c)

target_include_directories(unity PUBLIC unity)

add_executable(test_cs_usb_comms)
target_sources(test_cs_usb_comms PRIVATE
    test_cs_usb_comms.c
    ../lib/cs_usb_comms/src/cs_usb_comms.c
)
target_include_directories(test_cs_usb_comms PRIVATE
    ../lib/cs_usb_comms
    ../lib/cs_usb_comms/src
)
target_link_libraries(test_cs_usb_comms PRIVATE
    unity
)
add_test(NAME test_cs_usb_comms COMMAND test_cs_usb_comms)

add_executable(test_cs_led_ctrl)
target_sources(test_cs_led_ctrl PRIVATE
    test_cs_led_ctrl.c
    ../Core/Src/cs_led_ctrl.c
)
target_include_directories(test_cs_led_ctrl PRIVATE
    ../Core/Inc
)
target_link_libraries(test_cs_led_ctrl PRIVATE
    unity
)
target_compile_definitions(test_cs_led_ctrl PRIVATE LED_CTRL_NUM_LEDS=2)

add_test(NAME test_cs_led_ctrl COMMAND test_cs_led_ctrl)