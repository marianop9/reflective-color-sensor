#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdint.h>

void apply_filter(uint16_t *inputs, uint16_t *outputs, int input_count);

#endif // _FILTER_H_