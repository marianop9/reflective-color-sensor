add_library(unity STATIC unity/unity.c)

target_include_directories(unity PUBLIC unity)

# Helper function to add a Unity test executable + ctest entry
function(add_unity_test TEST_NAME)
  add_executable(${TEST_NAME} ${ARGN})
  target_link_libraries(${TEST_NAME} PRIVATE unity)

  if (NOT CMAKE_CROSSCOMPILING)
    target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()

  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

add_unity_test(test_ascii_comm_proto
    ${CMAKE_CURRENT_LIST_DIR}/ascii-comm-proto/test_serial_com.c
)

target_link_libraries(test_ascii_comm_proto PRIVATE ascii-comm-proto)

# add_executable(test_cs_usb_comms)
# target_sources(test_cs_usb_comms PRIVATE
#     test_cs_usb_comms.c
#     ../lib/cs_usb_comms/src/cs_usb_comms.c
# )
# target_include_directories(test_cs_usb_comms PRIVATE
#     ../lib/cs_usb_comms
#     ../lib/cs_usb_comms/src
# )
# target_link_libraries(test_cs_usb_comms PRIVATE
#     unity
# )
# add_test(NAME test_cs_usb_comms COMMAND test_cs_usb_comms)

# add_executable(test_cs_led_ctrl)
# target_sources(test_cs_led_ctrl PRIVATE
#     test_cs_led_ctrl.c
#     ../Core/Src/cs_led_ctrl.c
# )
# target_include_directories(test_cs_led_ctrl PRIVATE
#     ../Core/Inc
# )
# target_link_libraries(test_cs_led_ctrl PRIVATE
#     unity
# )
# target_compile_definitions(test_cs_led_ctrl PRIVATE LED_CTRL_NUM_LEDS=2)

# add_test(NAME test_cs_led_ctrl COMMAND test_cs_led_ctrl)
