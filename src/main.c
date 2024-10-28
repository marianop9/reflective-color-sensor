#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "lib/lcd/include/lcd.h"

// pins
#define LCD_PIN_RS 16
#define LCD_PIN_RW 17
#define LCD_PIN_ENABLE 18

#define LCD_PIN_D7 19
#define LCD_PIN_D6 20
#define LCD_PIN_D5 21
#define LCD_PIN_D4 22

void configure_pin(uint32_t pin, bool out) {
  gpio_init(pin);
  gpio_set_dir(pin, out);
}

void set_pin(uint32_t pin, bool value) {
  gpio_put(pin, value);
}

bool get_pin(uint32_t pin) {
  return gpio_get(pin);
}

// int led_pins[3] = {18, 19, 20};

int main() {
    stdio_init_all();
    adc_init();

    LCDAdapter_t* adapter = lcd_create(
        configure_pin, set_pin, get_pin, sleep_us,
        LCD_PIN_RS, LCD_PIN_RW, LCD_PIN_ENABLE,
        LCD_PIN_D7,  LCD_PIN_D6,  LCD_PIN_D5,  LCD_PIN_D4
    );

    lcd_init(adapter);
    lcd_hello_world(adapter);
        
    sleep_ms(500);
    printf("Hello world!\n");

    if (cyw43_arch_init()) {
        return -1;
    }

    // // Make sure GPIO is high-impedance, no pullups etc
    // adc_gpio_init(26);
    // // Select ADC input 0 (GPIO26)
    // adc_select_input(0);

    // uint gpio_mask = 1 << led_pins[0] | 1 << led_pins[1] | 1 << led_pins[2];
    // gpio_init_mask(gpio_mask);
    // gpio_set_dir_out_masked(gpio_mask);

    printf("blinking...\n");
    while (1) {
        // for (int i=0; i<3; i++) {
        //     gpio_put(led_pins[i], 1);
        //     sleep_ms(500);
        //     gpio_put(led_pins[i], 0);
        // }
        printf("led on\n");
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(500);
        printf("led off\n");
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(500);    }

    return 0;
}
