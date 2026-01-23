#include <stdint.h>

typedef void (*isr_cb)(void);

void init_adc(uint32_t adc_chan, uint16_t *_adc_buf, size_t adc_buf_len,
              isr_cb on_adc_finished_cb);

void start_adc_dma();

void stop_adc(bool is_irq);
