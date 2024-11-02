#include <stdio.h>

// Define the filter order
#define FILTER_ORDER 4

// Filter coefficients (hard-coded for this specific filter)
const double b[FILTER_ORDER + 1] = {9.00578213e-07, 3.60231285e-06, 5.40346928e-06, 3.60231285e-06, 9.00578213e-07};
const double a[FILTER_ORDER + 1] = {1.0, -3.83572813, 5.52054268, -3.53327341, 0.84847328};

// Buffers for previous input and output values
double x_buffer[FILTER_ORDER + 1] = {0.0};
double y_buffer[FILTER_ORDER + 1] = {0.0};

// Function to apply the Butterworth filter to a single sample
double butterworth_filter(double input) {
    // Shift previous inputs and outputs to the right
    for (int i = FILTER_ORDER; i > 0; i--) {
        x_buffer[i] = x_buffer[i - 1];
        y_buffer[i] = y_buffer[i - 1];
    }
    // Update current input
    x_buffer[0] = input;

    // Compute the current output using the difference equation
    double output = 0.0;
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

void apply_filter(double* inputs, double* outputs, int input_count) {
    for (int i=0; i < input_count; i++) {
        outputs[i] = butterworth_filter(inputs[i]);
    }
}