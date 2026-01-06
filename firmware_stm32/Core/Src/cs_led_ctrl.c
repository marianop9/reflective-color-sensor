#include "cs_led_ctrl.h"

#ifndef LED_CTRL_NUM_LEDS
#error "Must define LED_CTRL_NUM_LEDS"
#endif
#define LED_CTRL_RESET_PADDING 50
#define LED_CTRL_DMA_BUFFER_LEN (LED_CTRL_NUM_LEDS * 24 + LED_CTRL_RESET_PADDING)
// #define LED_CTRL_PWM_FREQ_KHZ 800 // kHZ

uint16_t led_ctrl_dma_buffer[LED_CTRL_DMA_BUFFER_LEN] = {0};

static uint32_t _pwm_max_counter;
static uint16_t _pulse_low;
static uint16_t _pulse_high;

const uint16_t *led_ctrl_get_buffer()
{
    return led_ctrl_dma_buffer;
}

uint16_t led_ctrl_get_buffer_len()
{
    return sizeof(led_ctrl_dma_buffer) / sizeof(led_ctrl_dma_buffer[0]);
}

void led_ctrl_init(uint16_t pwm_max_counter)
{
    // ARR is a 16-bit value
    _pwm_max_counter = pwm_max_counter;
    /* 72MHz clock
    T_pwm = 1.5us -> ARR=(108-1)
    From WS2812B datasheet:
    - T0H < 0.65us (typ. 0.4us)
    - T1H > 0.65us (typ. 0.8us)
    - T0L/T1L > 0.45us
    - Treset > 6u
    */
    _pulse_low = _pwm_max_counter * 2 / 7;
    _pulse_high = _pwm_max_counter * 2 / 3;

    // default value for LED bits is `_pulse_low`
    for (int i = 0; i < LED_CTRL_NUM_LEDS*24; i++)
    {
        led_ctrl_dma_buffer[i] = _pulse_low;
    }
    
}

int led_ctrl_set_buffer2(uint32_t led_num, uint32_t rgb)
{
    uint8_t r = rgb >> 16;
    uint8_t g = (rgb >> 8) & 0xff;
    uint8_t b = rgb & 0xff;
    return led_ctrl_set_buffer(led_num, r, g, b);
}

/* Sets the control buffer for a specific LED. If `led_num` == 0, all LEDs are
 * set to the specified color.
 */
int led_ctrl_set_buffer(uint32_t led_num,
                        uint8_t r,
                        uint8_t g,
                        uint8_t b)
{
    if (led_num > LED_CTRL_NUM_LEDS)
    {
        return -1;
    }

    if (led_num == 0)
    {
        for (int i = 0; i < LED_CTRL_NUM_LEDS; i++)
        {
            uint8_t offset = 24 * i;
            for (int i = 0; i < 8; i++)
            {
                led_ctrl_dma_buffer[offset + i] = (g & (1 << (7 - i))) ? _pulse_high : _pulse_low;
                led_ctrl_dma_buffer[offset + 8 + i] = (r & (1 << (7 - i))) ? _pulse_high : _pulse_low;
                led_ctrl_dma_buffer[offset + 16 + i] = (b & (1 << (7 - i))) ? _pulse_high : _pulse_low;
            }
        }
    }
    else
    {
        uint8_t offset = 24 * (led_num - 1);
        for (int i = 0; i < 8; i++)
        {
            led_ctrl_dma_buffer[offset + i] = (g & (1 << (7 - i))) ? _pulse_high : _pulse_low;
            led_ctrl_dma_buffer[offset + 8 + i] = (r & (1 << (7 - i))) ? _pulse_high : _pulse_low;
            led_ctrl_dma_buffer[offset + 16 + i] = (b & (1 << (7 - i))) ? _pulse_high : _pulse_low;
        }
    }

    // for (int i = (24 * LEDCFG_NUM); i < DMA_BUFFER_LEN; i++)
    // {
    //     led_dma_buffer[i] = 0;
    // }
    // technically not necessary since array is initialized to {0}
    // memset(led_dma_buffer + 24 * LEDCFG_NUM, 0, LEDCFG_RESET_PADDING);

    return 0;
}
