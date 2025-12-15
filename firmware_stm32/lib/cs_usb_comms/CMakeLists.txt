add_library(cs_usb_comms OBJECT)

target_sources(cs_usb_comms PRIVATE src/cs_usb_comms.c)

target_include_directories(cs_usb_comms PUBLIC .)