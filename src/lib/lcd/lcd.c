#include "internal.h"
#include "lcd.h"
#include <stdint.h>

LCDAdapter_t* lcd_create(
    configure_gpio configure_gpio,
    set_gpio set_gpio,
    get_gpio get_gpio,
    delay_us delay_us,
    int pin_rs, int pin_rw, int pin_e,
    int pin_d7, int pin_d6, int pin_d5, int pin_d4
  ) {
  LCDAdapter_t *config = (LCDAdapter_t*)malloc(sizeof(LCDAdapter_t));

  config->configure_gpio = configure_gpio;
  config->set_gpio = set_gpio;
  config->get_gpio = get_gpio;
  config->delay_us = delay_us;

  config->pin_rs = pin_rs;
  config->pin_rw = pin_rw;
  config->pin_e = pin_e;
  config->pins_data[0] = pin_d4;
  config->pins_data[1] = pin_d5;
  config->pins_data[2] = pin_d6;
  config->pins_data[3] = pin_d7;

  configure_gpio(pin_rs, true);
  configure_gpio(pin_rw, true);
  configure_gpio(pin_e, true);
  
  configure_gpio(pin_d7, true);
  configure_gpio(pin_d6, true);
  configure_gpio(pin_d5, true);
  configure_gpio(pin_d4, true);

  return config;
}

void lcd_send(LCDAdapter_t* adapter, uint8_t value, enum LCD_MODE mode) {  
  // set mode
  uint32_t mode_value = mode == LCD_MODE_DATA
                      ? 1
                      : 0;
  adapter->set_gpio(adapter->pin_rs, mode_value);

  // set R-W
  adapter->set_gpio(adapter->pin_rw, 0);

  // write bits
  lcd_write4(adapter, value >> 4);
  lcd_write4(adapter, value);
}

void lcd_send4(LCDAdapter_t* adapter, uint8_t value, enum LCD_MODE mode) {
  // set mode
  uint32_t mode_value = mode == LCD_MODE_DATA
                      ? 1
                      : 0;
  adapter->set_gpio(adapter->pin_rs, mode_value);

  // set R-W
  adapter->set_gpio(adapter->pin_rw, 0);

  // write bits
  lcd_write4(adapter, value);
}

void lcd_init(LCDAdapter_t* adapter) {
  adapter->delay_us(40000);

  // set 4-bit mode
  lcd_send4(adapter, 0x02, LCD_MODE_INSTRUCTION);  
  adapter->delay_us(4500);
  
  // set 4-bit mode, 2 lines, 5x8 columns
  lcd_send(adapter, 0b00101000, LCD_MODE_INSTRUCTION);
  adapter->delay_us(150);

  lcd_send(adapter, 0b00101000, LCD_MODE_INSTRUCTION);
  adapter->delay_us(150);

  // display on
  lcd_send(adapter, 0x0e, LCD_MODE_INSTRUCTION);
  adapter->delay_us(150);

  // clear display
  lcd_send(adapter, 0x01, LCD_MODE_INSTRUCTION);
  adapter->delay_us(150);

  // set display right-to-left and no shift
  lcd_send(adapter, 0x06, LCD_MODE_INSTRUCTION);
  adapter->delay_us(150);
} 

void lcd_await_busy(LCDAdapter_t* adapter) {
  adapter->set_gpio(adapter->pin_rs, 0);
  adapter->set_gpio(adapter->pin_rw, 1);

  // busy flag is set in pin d7
  uint32_t busy_flag_pin = adapter->pins_data[3];

  // temporarly set d7 as input
  adapter->configure_gpio(busy_flag_pin, false);

  // loop until busy flag is set low
  
  adapter->set_gpio(adapter->pin_e, 1);
  while(adapter->get_gpio(busy_flag_pin)) { }

  adapter->set_gpio(adapter->pin_e, 0);

  // set d7 back to output
  adapter->configure_gpio(busy_flag_pin, true);
}

void lcd_write4(LCDAdapter_t* adapter, uint8_t val) {
  for (int i=0; i<4; i++) {
    adapter->set_gpio(adapter->pins_data[i], (val >> i) & 0x01);
  }

  lcd_pulse_enable(adapter);
}

void lcd_pulse_enable(LCDAdapter_t* adapter) {
  adapter->set_gpio(adapter->pin_e, 0);
  adapter->delay_us(100);
  adapter->set_gpio(adapter->pin_e, 1);
  adapter->delay_us(100);
  adapter->set_gpio(adapter->pin_e, 0);
  adapter->delay_us(100);
}

void lcd_command(LCDAdapter_t* adapter, uint8_t value) {
  lcd_await_busy(adapter);
  lcd_send(adapter, value, LCD_MODE_INSTRUCTION);
}

void lcd_data(LCDAdapter_t* adapter, uint8_t value) {
  lcd_await_busy(adapter);
  lcd_send(adapter, value, LCD_MODE_DATA);
}

void lcd_hello_world(LCDAdapter_t* adapter) {
  char* data = "hello lcd";

  for (char* p=data; *p != '\0'; p++) {
    lcd_data(adapter, *p);  
  }
}

void lcd_clear(LCDAdapter_t* adapter) {
  lcd_command(adapter, 0x01);
}

void lcd_home(LCDAdapter_t* adapter) {
  lcd_command(adapter, 0x02);
}

void lcd_set_cursor(LCDAdapter_t* adapter, bool show, bool blink) {
  uint8_t cmd = show << 1 | blink;
  lcd_command(adapter, cmd);
}

// writes a null-terminated string
void lcd_print(LCDAdapter_t* adapter, char* str) {
  for (char* p=str; *p != '\0'; p++) {
    lcd_data(adapter, *p);
  }
}

void lcd_set_position(LCDAdapter_t *adapter, uint8_t row, uint8_t col) {
  if (row != 0 && row != 1) {
    return;
  }
  if (col < 0 && row > 15) {
    return;
  }

  // 2-line mode:
  // line 1: 0x00 to 0x27
  // line 2: 0x40 to 0x67
  // ddram can hold up to 80 characters (40/line)

  uint8_t pos = (row * 0x40) | col;

  uint8_t cmd = 0b10000000 | pos;
  lcd_command(adapter, cmd);
}
