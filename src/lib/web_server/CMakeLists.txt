add_executable(web_server web_server.c)

target_link_libraries(web_server
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    pico_lwip_http
    pico_httpd_content
)

target_include_directories(web_server PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_enable_stdio_usb(web_server 1)
pico_enable_stdio_uart(web_server 0)

pico_add_extra_outputs(web_server)

set(WIFI_SSID "" CACHE STRING "Wi-Fi SSID")
set(WIFI_PASSWORD "" CACHE STRING "Wi-Fi password")

if(WIFI_SSID STREQUAL "")
    message(FATAL_ERROR "missing WiFi SSID")
elseif(WIFI_PASSWORD STREQUAL "")
    message(FATAL_ERROR "missing WiFi password")
endif()

add_compile_definitions(web_server WIFI_SSID="${WIFI_SSID}" WIFI_PASSWORD="${WIFI_PASSWORD}")


# Crea una librería especial para el contenido estático.
# Se debe linkear con el ejecutable/librería!!!!
pico_add_library(pico_httpd_content NOFLAG)
pico_set_lwip_httpd_content(pico_httpd_content INTERFACE
        # static source files:
        ${CMAKE_CURRENT_LIST_DIR}/content/hello.html
        ${CMAKE_CURRENT_LIST_DIR}/content/index.shtml
)