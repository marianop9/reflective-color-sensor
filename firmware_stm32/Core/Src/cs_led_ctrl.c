#include "cs_led_ctrl.h"

uint16_t led_ctrl_dma_buffer[LED_CTRL_DMA_BUFFER_LEN] = {0};

static uint32_t _pwm_max_counter;

void led_ctrl_init(uint16_t pwm_max_counter)
{
    _pwm_max_counter = pwm_max_counter;
}

int led_ctrl_set_buffer(uint32_t index,
                        uint8_t r,
                        uint8_t g,
                        uint8_t b)
{
    if (index >= LED_CTRL_NUM_LEDS)
    {
        return -1;
    }

    /* 72MHz clock
    T_pwm = 1.5us -> ARR=(108-1)
    From WS2812B datasheet:
    - T0H < 0.65us (typ. 0.4us)
    - T1H > 0.65us (typ. 0.8us)
    - T0L/T1L > 0.45us
    - Treset > 6u
    */
    // ARR is a 16-bit value
    uint16_t pulse_low = _pwm_max_counter * 2 / 7;
    uint16_t pulse_high = _pwm_max_counter * 2 / 3;

    uint8_t offset = 24 * index;
    for (int i = 0; i < 8; i++)
    {
        led_ctrl_dma_buffer[offset + i] = (g & (1 << (7 - i))) ? pulse_high : pulse_low;
        led_ctrl_dma_buffer[offset + 8 + i] = (r & (1 << (7 - i))) ? pulse_high : pulse_low;
        led_ctrl_dma_buffer[offset + 16 + i] = (b & (1 << (7 - i))) ? pulse_high : pulse_low;
    }

    // for (int i = (24 * LEDCFG_NUM); i < DMA_BUFFER_LEN; i++)
    // {
    //     led_dma_buffer[i] = 0;
    // }
    // technically not necessary since array is initialized to {0}
    // memset(led_dma_buffer + 24 * LEDCFG_NUM, 0, LEDCFG_RESET_PADDING);

    return 0;
}
