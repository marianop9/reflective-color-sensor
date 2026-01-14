add_library(cs_web_server web_server.c)

target_include_directories(cs_web_server 
    # INTERFACE ${CMAKE_CURRENT_LIST_DIR}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/
)

target_link_libraries(cs_web_server
    pico_stdlib
    pico_util
    pico_cyw43_arch_lwip_poll
    pico_lwip_http
    httpd_content
)

set(WIFI_SSID "" CACHE STRING "Wi-Fi SSID")
set(WIFI_PASSWORD "" CACHE STRING "Wi-Fi password")

if(WIFI_SSID STREQUAL "" OR WIFI_PASSWORD STREQUAL "")
    message(WARNING "missing WiFi SSID or Password")
else()
    message(STATUS "Wi-Fi configured: ${WIFI_SSID} - ${WIFI_PASSWORD}")
    target_compile_definitions(cs_web_server PRIVATE WIFI_SSID=${WIFI_SSID} WIFI_PASSWORD=${WIFI_PASSWORD})
endif()


# Crea una librería especial para el contenido estático.
# Se debe linkear con el ejecutable/librería!!!!
pico_add_library(httpd_content NOFLAG)
pico_set_lwip_httpd_content(httpd_content INTERFACE
        # static source files:
        ${CMAKE_CURRENT_LIST_DIR}/content/hello.html
        ${CMAKE_CURRENT_LIST_DIR}/content/index.shtml
        ${CMAKE_CURRENT_LIST_DIR}/content/404.html
        
        ${CMAKE_CURRENT_LIST_DIR}/content/color_response.json
        ${CMAKE_CURRENT_LIST_DIR}/content/action_response.json
)

add_executable(example_server example_server.c)

target_link_libraries(example_server 
    pico_multicore
    cs_web_server
)

pico_enable_stdio_usb(example_server 1)
pico_enable_stdio_uart(example_server 0)

pico_add_extra_outputs(example_server)