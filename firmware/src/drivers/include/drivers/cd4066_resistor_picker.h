#include <stdbool.h>
#include <stdint.h>

bool cd4066_init(uint32_t *values, uint32_t *pins, uint32_t pin_count);
void cd4066_reset();
bool cd4066_next();
bool cd4066_prev();
uint32_t cd4066_get_current_resistor_ohms();
