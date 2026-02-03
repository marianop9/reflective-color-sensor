/**
 * Code based on example from "pico-examples" repo.
 * https://github.com/raspberrypi/pico-examples/blob/master/pio/ws2812/ws2812.c
 */

#include "drivers/ws2812.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

/**
 * NOTE:
 *  Take into consideration if your WS2812 is a RGB or RGBW variant.
 *
 */
#define WS2812_IS_RGBW false
#define WS2812_NUM_PIXELS 2

/** Pixel data in the expected transmission format:
 * Bits [31:8] 24-bit GRB (sent MSB-first)
 * Bits [7: 0] Ignored
 */
static uint32_t pixels[WS2812_NUM_PIXELS] = {0};
static PIO pio;
static uint sm;
static uint offset;

void ws2812_sync_pixels() {
    for (int i = 0; i < WS2812_NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, pixels[i]);
    }
}

bool ws2812_set_pixel(uint32_t led_num, uint32_t rgb) {
    uint8_t r = rgb >> 16;
    uint8_t g = (rgb >> 8) & 0xff;
    uint8_t b = rgb & 0xff;

    return ws2812_set_pixel2(led_num, r, g, b);
}

bool ws2812_set_pixel2(uint32_t led_num, uint8_t r, uint8_t g, uint8_t b) {
    if (led_num >= WS2812_NUM_PIXELS) {
        return false;
    }

    if (led_num == 0) {
        for (; led_num < WS2812_NUM_PIXELS; led_num++) {
            pixels[led_num] = ((uint32_t)(g) << 24) | ((uint32_t)(r) << 16) |
                              ((uint32_t)(b) << 8);
        }
    } else {
        pixels[led_num] = ((uint32_t)(g) << 24) | ((uint32_t)(r) << 16) |
                          ((uint32_t)(b) << 8);
    }

    return true;
}

void ws2812_pixels_off() {
    for (int i = 0; i < WS2812_NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, 0);
    }
}

bool ws2812_init(uint32_t ws2812_pin) {
    // Check the pin is compatible with the platform:
    // Attempting to use a pin>=32 on a platform that does not support it
    if (ws2812_pin >= NUM_BANK0_GPIOS) {
        return false;
    }

    // This will find a free pio and state machine for our program and load it
    // for us We use pio_claim_free_sm_and_add_program_for_gpio_range
    // (for_gpio_range variant) so we will get a PIO instance suitable for
    // addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
        &ws2812_program, &pio, &sm, &offset, ws2812_pin, 1, true);

    if (success) {
        ws2812_program_init(pio, sm, offset, ws2812_pin, 800000,
                            WS2812_IS_RGBW);
    }

    return success;
}

void ws2812_deinit() {
    // This will free resources and unload our program
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}