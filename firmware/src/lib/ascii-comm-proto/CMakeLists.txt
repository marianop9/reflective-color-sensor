add_library(ascii-comm-proto OBJECT)

target_sources(ascii-comm-proto PRIVATE src/serial_com.c)

target_include_directories(ascii-comm-proto PUBLIC include)