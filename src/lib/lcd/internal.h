#include "include/lcd.h"

void lcd_pulse_enable(LCDAdapter_t* adapter);

void lcd_write4(LCDAdapter_t* adapter, uint8_t val);

void lcd_await_busy(LCDAdapter_t* adapter);

void lcd_send(LCDAdapter_t* adapter, uint8_t value, enum LCD_MODE mode);

void lcd_send4(LCDAdapter_t* adapter, uint8_t value, enum LCD_MODE mode);
