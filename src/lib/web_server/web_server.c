#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/apps/httpd.h"
#include "lwip/init.h"
#include "lwip/tcp.h"

void print_ip_address() {
    const ip4_addr_t *ip = netif_ip4_addr(netif_default); // Get the IP address
    if (ip) {
        printf("Pico W IP Address: %s\n", ip4addr_ntoa(ip));
    } else {
        printf("Failed to get IP address\n");
    }
}

void set_led(int on) { cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on); }

int wifi_connect(const char *ssid, const char *pwd) {
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    set_led(1);
    sleep_ms(4000);
    set_led(0);

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

const char *handleIndex(int iIndex, int iNumParams, char *pcParam[],
                        char *pcValue[]) {
    return "/index.shtml";
}

const char *ssi_tags[] = {
    "hexcode",
};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
    return snprintf(pcInsert, iInsertLen, "#0000ff");
}

int main() {
    stdio_init_all();

    wifi_connect(WIFI_SSID, WIFI_PASSWORD);

    // struct tcp_pcb *pcb = tcp_new();
    // if (pcb == NULL) {
    //     printf("Failed to create PCB\n");
    //     return 1;
    // }

    // tcp_bind(pcb, IPADDR_ANY, 80);

    // pcb = tcp_listen(pcb);
    // printf("HTTP server listening on port 80\n");

    httpd_init();

    tCGI handlers[] = {
        {.pcCGIName = "/", .pfnCGIHandler = handleIndex},
    };
    http_set_cgi_handlers(handlers, sizeof(handlers) / sizeof(handlers[0]));

    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));

    set_led(1);

    while (true) {
        cyw43_arch_poll();
        sleep_ms(500);
    }

    printf("exiting...\n");

    cyw43_arch_deinit();
    return 0;
}