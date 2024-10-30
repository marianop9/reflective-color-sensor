#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <stdio.h>

#include "lcd.h"

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
enum Colors {
  R = 0, G, B
};
char colorNames[] = {'R', 'G', 'B'};

void update_display(LCDAdapter_t *adapter, enum Colors color, uint8_t val) {
  // 1era linea muestra el valor hex directo
  uint8_t hex_col = 5;
  lcd_set_position(adapter, 0, hex_col + color*2);

  char hex_buffer[3];
  sprintf(hex_buffer, "%02x", val);

  lcd_print(adapter, hex_buffer);

  lcd_set_position(adapter, 1, color*6);

  int rel_val = val * 100 / 256;
  char rel_buffer[3];
  sprintf(rel_buffer, "%c:%d", colorNames[color], rel_val);

  lcd_print(adapter, rel_buffer); 
}

int main() {
    stdio_init_all();
    adc_init();

    LCDAdapter_t* adapter = lcd_create(
        configure_pin, set_pin, get_pin, sleep_us,
        LCD_PIN_RS, LCD_PIN_RW, LCD_PIN_ENABLE,
        LCD_PIN_D7,  LCD_PIN_D6,  LCD_PIN_D5,  LCD_PIN_D4
    );

    lcd_init(adapter);
    update_display(adapter, R, 255);
    update_display(adapter, G, 0);
    update_display(adapter, B, 64);

        
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
