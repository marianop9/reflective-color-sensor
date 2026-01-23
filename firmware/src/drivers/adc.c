#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"

#include "drivers/adc.h"

static uint32_t adc_dma_chan = 0;
static uint16_t *padc_buf = NULL;

void init_adc(uint32_t adc_chan, uint16_t *_adc_buf, size_t adc_buf_len,
              isr_cb on_adc_finished_cb) {
    // Initalize adc and select current channels
    adc_gpio_init(ADC_BASE_PIN + adc_chan);
    adc_init();
    adc_select_input(adc_chan);

    padc_buf = _adc_buf;

    /** ADC starts a conversion every (clk_div + 1) clock cycles.
     *  ADC is paced by 48 MHz clock.
     *  for clk_div=47999:
     *      clk_div+1=48e3
     *      fs = 48e6 / (clk_div+1) = 1 ksps
     */
    const int fs = 500;
    const uint32_t freq_adc_hz = clock_get_hz(clk_adc);

    const int div = (freq_adc_hz / fs) - 1;

    adc_set_clkdiv(div);
    adc_fifo_setup(
        true,  // Write each completed conversion to the sample FIFO
        true,  // Enable DMA data request (DREQ)
        1,     // DREQ (and IRQ) asserted when at least 1 sample present
        false, // We won't see the ERR bit because of 8 bit reads; disable.
        false  // Don't! Shift each sample to 8 bits when pushing to FIFO
    );

    adc_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config_t dma_cfg = dma_channel_get_default_config(adc_dma_chan);

    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);

    dma_channel_configure(adc_dma_chan, &dma_cfg,
                          padc_buf,      // dst
                          &adc_hw->fifo, // src (the adc FIFO)
                          adc_buf_len,   // number of transfers
                          false          // start inmediately
    );

    // tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(adc_dma_chan, true);
    // configure IRQ handler
    irq_set_exclusive_handler(DMA_IRQ_0, on_adc_finished_cb);
    irq_set_enabled(DMA_IRQ_0, true);
}

void start_adc_dma() {
    dma_channel_set_write_addr(adc_dma_chan, padc_buf, true);
    adc_run(true);
}

void stop_adc(bool is_irq) {
    if (is_irq && dma_hw->ints0 & (1u << adc_dma_chan)) {
        dma_channel_acknowledge_irq0(adc_dma_chan);
    }
    adc_run(false);
}

// void led_task(void *arg) {
//     while (1) {
//         gpio_put(PICO_DEFAULT_LED_PIN, true);
//         vTaskDelay(pdMS_TO_TICKS(2000));

//         // puts("Start ADC");
//         sample_adc();
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

//         // puts("Build Resp");
//         acp_response_t resp = {
//             .type = ACP_RESP_DATA,
//             .payload_len_bytes =
//                 ADC_NUM_SAMPLES * sizeof(adc_buffer[0]) /
//                 sizeof(tx_buf[0]),
//         };

//         // puts("Format Resp");
//         for (size_t i = 0; i < ADC_NUM_SAMPLES; i++) {
//             format_u16_le(&resp.payload[2 * i], adc_buffer[i]);
//         }

//         tx_buf_len = acp_format_response(tx_buf, &resp);

//         size_t n =
//         fwrite(tx_buf, sizeof(tx_buf[0]), tx_buf_len, stdout);
//         if (n != tx_buf_len)
//             error_blink();
//         // printf("Message: %d, Sent %d\n", tx_buf_len, n);

//         gpio_put(PICO_DEFAULT_LED_PIN, false);
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
// }

// int main() {
//     if (!stdio_init_all())
//         error_blink();

//     gpio_init(PICO_DEFAULT_LED_PIN);
//     gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
//     gpio_put(PICO_DEFAULT_LED_PIN, 0);

//     init_adc();

//     BaseType_t created =
//         xTaskCreate(led_task, "led_task", configMINIMAL_STACK_SIZE, NULL,
//         10,
//                     &led_task_handle);
//     if (created != pdPASS) {
//         gpio_put(PICO_DEFAULT_LED_PIN, 1);
//     }

//     vTaskStartScheduler();
//     return 1;
// }