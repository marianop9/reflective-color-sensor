/* Aplicación ejemplo usando un servidor HTTP con lwIP
 */
#include "web_server.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/apps/httpd.h"
#include "lwip/init.h"
#include "lwip/tcp.h"

#include <stdio.h>

#define FLAG 99

// replaced with semaphore
// volatile bool triggered = false;
semaphore_t *_trigger_sem;

queue_t *_data_queue;

const char *get_ip_address() {
    const ip4_addr_t *ip = netif_ip4_addr(netif_default); // Get the IP address

    if (ip)
        return ip4addr_ntoa(ip);

    printf("failed to get ip address\n");
    return NULL;
}

void set_led(int on) { cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on); }

// valor inicial (medio gris)
char last_hexcode[7] = "cecece";

void set_last_measurement() {
    cs_web_server_data_t received;
    if (queue_try_remove(_data_queue, &received)) {
        // uint8_t r = (received >> 16) & 0xff;
        // uint8_t g = (received >> 8) & 0xff;
        // uint8_t b = received & 0xff;
        // sprintf(last_hexcode, "%02X%02X%02X", r, g, b);
        sprintf(last_hexcode, "%06X", received);
    }
}

// no debe estar ejecutando este handler sino otro por defecto para la ruta "/"
// lwIP tiene una lista de nombres predeterminados, antes de buscar el archivo
// pedido en el CGI, se fija si la ruta coincide con algun nombre predeterminado
// ver NUM_DEFAULT_FILENAMES
// ESTE HANDLER NUNCA SE EJECUTA
const char *handleIndex(int iIndex, int iNumParams, char *pcParam[],
                        char *pcValue[]) {
    return "/index.shtml";
}

const char *handleLastMeasurement(int iIndex, int iNumParams, char *pcParam[],
                                  char *pcValue[]) {
    set_last_measurement();
    return "/color_response.json";
}

const char *handleTrigger(int iIndex, int iNumParams, char *pcParam[],
                          char *pcValue[]) {
    // libera el semaforo
    sem_release(_trigger_sem);

    return "/action_response.json";
}

const char *ssi_tags[] = {
    "hexcode",
    "status",
};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
    u16_t written = 0;
    switch (iIndex) {
    case 0: {
        written = snprintf(pcInsert, iInsertLen, "#%s", last_hexcode);
        break;
    }
    case 1: {
        written = snprintf(pcInsert, iInsertLen, "triggered");
        break;
    }
    }

    return written;
}

tCGI cgi_handlers[] = {
    {.pcCGIName = "/", .pfnCGIHandler = handleIndex},
    {.pcCGIName = "/last-measurement", .pfnCGIHandler = handleLastMeasurement},
    {.pcCGIName = "/trigger-measurement", .pfnCGIHandler = handleTrigger},
};

int web_server_init(queue_t *pdata_queue, semaphore_t *ptrigger_sem) {
    _data_queue = pdata_queue;
    _trigger_sem = ptrigger_sem;

    queue_init(_data_queue, sizeof(cs_web_server_data_t), 1);

    sem_init(_trigger_sem, 0, 1);

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    set_led(1);
    sleep_ms(2000);
    set_led(0);

    cyw43_arch_enable_sta_mode();

    printf("connecting to %s\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("failed to connect\n");
        return 1;
    }

    const char *ipaddr = get_ip_address();
    if (ipaddr == NULL)
        return 1;

    printf("Pico W server running at IP Address: %s\n", ipaddr);
    httpd_init();

    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));

    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));

    set_led(1);
}

void web_server_poll() {
    printf("starting web server poll\n");
    while (true) {
        cyw43_arch_poll();
        sleep_ms(500);
    }
}

// TODO!

// move web_server to run on a separate core

// notify sensor core when a new measurement is requested

// receive and persist last measurement from sensor core

// configure ssi to retrieve the last measurement

// webpage should delay > 1s after measurement is requested and then ask for the
// last measurement