#include "lcd.h"

void lcd_pulse_enable(LCDAdapter_t* adapter);

void lcd_write4(LCDAdapter_t* adapter, uint8_t val);

void lcd_await_busy(LCDAdapter_t* adapter);

void lcd_send(LCDAdapter_t* adapter, uint8_t value, enum LCD_MODE mode);

void lcd_send4(LCDAdapter_t* adapter, uint8_t value, enum LCD_MODE mode);


/*
// LCD pins
#define LCD_PIN_RS 16
#define LCD_PIN_RW 17
#define LCD_PIN_ENABLE 18

#define LCD_PIN_D7 19
#define LCD_PIN_D6 20
#define LCD_PIN_D5 21
#define LCD_PIN_D4 22 
*/
