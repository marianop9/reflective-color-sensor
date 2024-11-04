#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/apps/httpd.h"
#include "lwip/init.h"
#include "lwip/tcp.h"

// move to compile-time definition
#define WIFI_SSID "Fibertel WiFi709 2.4GHz"
#define WIFI_PWD "01429015004"

void print_ip_address() {
    const ip4_addr_t *ip = netif_ip4_addr(netif_default); // Get the IP address
    if (ip) {
        printf("Pico W IP Address: %s\n", ip4addr_ntoa(ip));
    } else {
        printf("Failed to get IP address\n");
    }
}

int wifi_connect(const char *ssid, const char *pwd) {
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("connecting...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, pwd, CYW43_AUTH_WPA2_AES_PSK,
                                           10000)) {
        printf("failed to connect\n");
        return 1;
    }

    printf("connected!\n\n");
    print_ip_address();
}

const char *handleHelloWorld(int iIndex, int iNumParams, char *pcParam[],
                             char *pcValue[]) {
    return "/hello.html";
}

int main() {
    stdio_init_all();
    sleep_ms(4000);

    wifi_connect(WIFI_SSID, WIFI_PWD);

    // struct tcp_pcb *pcb = tcp_new();
    // if (pcb == NULL) {
    //     printf("Failed to create PCB\n");
    //     return 1;
    // }

    // tcp_bind(pcb, IPADDR_ANY, 80);

    // pcb = tcp_listen(pcb);
    // printf("HTTP server listening on port 80\n");

    tCGI handlers[] = {
        {.pcCGIName = "/", .pfnCGIHandler = handleHelloWorld},
    };

    httpd_init();
    http_set_cgi_handlers(handlers, 2);

    while (true) {
        cyw43_arch_poll();
        sleep_ms(500);
    }

    printf("exiting...\n");

    cyw43_arch_deinit();
    return 0;
}