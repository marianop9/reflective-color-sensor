#include "drivers/cd4066_resistor_picker.h"

#include "hardware/gpio.h"

#define CD4066_MAX_CHANNELS 4

static uint32_t resistor_values[CD4066_MAX_CHANNELS] = {0};
static uint32_t enable_pins[CD4066_MAX_CHANNELS] = {0};
static uint32_t num_channels = 0;
// currently selected channel
static uint32_t current_chan_idx = 0;
// mask indicating all configured enable pins
static uint32_t enable_pins_mask = 0;

void (*delay_fn)(void) = NULL;

void update_current_pin() {
    uint32_t pin = enable_pins[current_chan_idx];

    // make before break
    gpio_put(pin, true);
    if (delay_fn != NULL)
        delay_fn();

    gpio_clr_mask(enable_pins_mask ^ (1 << pin));
}

/** Initialize driver
 * `values` indicates the resistor values for each channel.
 * `pins` are the enable pins for each channel.
 * `pin_count` is the number of channels in use. `values` and `pins` should be
 * of this size.
 */
bool cd4066_init(uint32_t *values, uint32_t *pins, uint32_t pin_count,
                 void (*make_before_break_delay_fn)(void)) {
    if (pin_count > CD4066_MAX_CHANNELS)
        return false;

    num_channels = pin_count;
    for (size_t i = 0; i < num_channels; i++) {
        resistor_values[i] = values[i];
        enable_pins[i] = pins[i];

        gpio_init(enable_pins[i]);
        gpio_set_dir(enable_pins[i], GPIO_OUT);
        gpio_pull_down(enable_pins[i]);

        enable_pins_mask |= (1 << enable_pins[i]);
    }

    // start in known state
    current_chan_idx = 0;
    gpio_put(enable_pins[0], true);

    delay_fn = make_before_break_delay_fn;

    return true;
}

/** Resets to the first channel. */
void cd4066_reset() {
    current_chan_idx = 0;
    update_current_pin();
}

/** Drives the next channel. Exits early and returns `false` if already at the
 * last channel. */
bool cd4066_next() {
    current_chan_idx += 1;
    if (current_chan_idx >= num_channels) {
        return false;
    }

    update_current_pin();

    return true;
}

/** Drives the previous channel. Exits early and returns `false` if already at
 * the first channel. */
bool cd4066_prev() {
    if (current_chan_idx == 0) {
        return false;
    }
    current_chan_idx -= 1;

    update_current_pin();

    return true;
}

uint32_t cd4066_get_current_resistor_ohms() {
    return resistor_values[current_chan_idx];
}