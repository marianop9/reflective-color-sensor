#include <stdbool.h>
#include <stdint.h>

bool ws2812_init(uint32_t ws2812_pin);
void ws2812_deinit();

bool ws2812_set_pixel(uint32_t led_num, uint32_t rgb);
bool ws2812_set_pixel2(uint32_t led_num, uint8_t r, uint8_t g, uint8_t b);

void ws2812_sync_pixels();
void ws2812_pixels_off();