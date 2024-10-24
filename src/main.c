#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "hardware/adc.h"

int led_pins[3] = {18, 19, 20};

int main() {
    stdio_init_all();
    adc_init();

    if (cyw43_arch_init()) {
        return -1;
    }

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    uint gpio_mask = 1 << led_pins[0] | 1 << led_pins[1] | 1 << led_pins[2];
    gpio_init_mask(gpio_mask);
    gpio_set_dir_out_masked(gpio_mask);

    printf("blinking...\n");
    while (1) {
        for (int i=0; i<3; i++) {
            gpio_put(led_pins[i], 1);
            sleep_ms(500);
            gpio_put(led_pins[i], 0);
        }
    }

    return 0;
}
