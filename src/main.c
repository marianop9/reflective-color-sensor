#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    sleep_ms(500);
    printf("Hello world!\n");

    if (cyw43_arch_init()) {
        return -1;
    }

    printf("blinking...\n");
    while (1) {
        printf("led on\n");
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(500);
        printf("led off\n");
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(500);
    }

    return 0;
}
