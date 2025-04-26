#include "filter.h"

// Butterworth LP filter - Sample frequency fs=500 Hz

// Define the filter order
#define FILTER_ORDER 2

// Filter coefficients (hard-coded for this specific filter)
const float b[FILTER_ORDER + 1] = {0.00362991, 0.00725982, 0.00362991};
const float a[FILTER_ORDER + 1] = {1., -1.82248576, 0.8370054};

// Buffers for previous input and output values
float x_buffer[FILTER_ORDER + 1] = {0.};
float y_buffer[FILTER_ORDER + 1] = {0.};

// Function to apply the Butterworth filter to a single sample
float butterworth_filter(float input) {
    // Shift previous inputs and outputs to the right
    for (int i = FILTER_ORDER; i > 0; i--) {
        x_buffer[i] = x_buffer[i - 1];
        y_buffer[i] = y_buffer[i - 1];
    }
    // Update current input
    x_buffer[0] = input;

    // Compute the current output using the difference equation
    float output = 0.0;
    for (int i = 0; i <= FILTER_ORDER; i++) {
        output += b[i] * x_buffer[i];
    }
    for (int i = 1; i <= FILTER_ORDER; i++) {
        output -= a[i] * y_buffer[i];
    }

    // Update the current output in the buffer
    y_buffer[0] = output;

    return output;
}

void apply_filter(uint16_t *inputs, uint16_t *outputs, int input_count) {
    // clear buffers
    for (int i = 0; i < FILTER_ORDER + 1; i++) {
        x_buffer[i] = 0.f;
        y_buffer[i] = 0.f;
    }

    for (int i = 0; i < input_count; i++) {
        outputs[i] = (uint16_t)butterworth_filter((float)inputs[i]);
    }
}