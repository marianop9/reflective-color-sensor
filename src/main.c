#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
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

int main() {
    stdio_init_all();

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
