#ifndef _CS_LED_CTRL_
#define _CS_LED_CTRL_

#include <stdint.h>

const uint16_t *led_ctrl_get_buffer();

uint16_t led_ctrl_get_buffer_len();

void led_ctrl_init(uint16_t pwm_max_counter);

int led_ctrl_set_buffer2(uint32_t led_num, uint32_t rgb);

int led_ctrl_set_buffer(
    uint32_t led_num,
    uint8_t r,
    uint8_t g,
    uint8_t b);

#endif // _CS_LED_CTRL_