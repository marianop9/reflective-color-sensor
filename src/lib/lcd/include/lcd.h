#ifndef LCD_H_
#define LCD_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*configure_gpio)(uint32_t pin_num, bool output);

typedef void (*set_gpio)(uint32_t pin_num, bool value);

typedef bool (*get_gpio)(uint32_t pin_num);

typedef void (*delay_us)(uint64_t us);

typedef struct {
  configure_gpio configure_gpio;
  set_gpio set_gpio;
  get_gpio get_gpio;
  delay_us delay_us;

  int pins_data[4];
  int pin_rw;
  int pin_rs;
  int pin_e;
} LCDAdapter_t;

enum LCD_MODE {
  LCD_MODE_INSTRUCTION, 
  LCD_MODE_DATA
};

LCDAdapter_t* lcd_create(
    configure_gpio configure_gpio,
    set_gpio set_gpio,
    get_gpio get_gpio,
    delay_us delay_us,
    int pin_rs, int pin_rw, int pin_e,
    int pin_d7, int pin_d6, int pin_d5, int pin_d4
  );

void lcd_init(LCDAdapter_t* adapter);

void lcd_command(LCDAdapter_t* adapter, uint8_t value);

void lcd_data(LCDAdapter_t* adapter, uint8_t value);

void lcd_hello_world(LCDAdapter_t* adapter);

void lcd_clear(LCDAdapter_t* adapter);

void lcd_home(LCDAdapter_t* adapter);

void lcd_set_cursor(LCDAdapter_t* adapter, bool show, bool blink);

void lcd_write(LCDAdapter_t* adapter, char* str);

#endif
