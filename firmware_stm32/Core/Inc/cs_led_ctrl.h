#ifndef _CS_LED_CTRL_
#define _CS_LED_CTRL_

#include <stdint.h>

#define LED_CTRL_PWM_FREQ_KHZ 800 // kHZ
#define LED_CTRL_NUM_LEDS 1
#define LED_CTRL_RESET_PADDING 50
#define LED_CTRL_DMA_BUFFER_LEN (LED_CTRL_NUM_LEDS * 24 + LED_CTRL_RESET_PADDING)

extern uint16_t led_ctrl_dma_buffer[LED_CTRL_DMA_BUFFER_LEN];

void led_ctrl_init(uint16_t pwm_max_counter);

int led_ctrl_set_buffer2(uint32_t index, uint32_t rgb);

int led_ctrl_set_buffer(
    uint32_t index,
    uint8_t r,
    uint8_t g,
    uint8_t b);

#endif // _CS_LED_CTRL_