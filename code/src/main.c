#include "filter.h"
#include "web_server.h"

#include "hardware/adc.h"
#include "hardware/dma.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include <stdio.h>

#define FLAG 99

#define ADC_CHAN 2

#define LED_PIN_R 18 // 630 nm
#define LED_PIN_G 19 // 520 nm
#define LED_PIN_B 20 // 470 nm

void configure_pin(uint32_t pin, bool out) {
    gpio_init(pin);
    gpio_set_dir(pin, out);
}

void set_pin(uint32_t pin, bool value) { gpio_put(pin, value); }

bool get_pin(uint32_t pin) { return gpio_get(pin); }

int led_pins[3] = {LED_PIN_R, LED_PIN_G, LED_PIN_B};
enum Colors { R = 0, G, B };
char colorNames[] = {'R', 'G', 'B'};

#define ADC_READ_COUNT 100
uint16_t adc_r_buffer[ADC_READ_COUNT];
uint16_t adc_g_buffer[ADC_READ_COUNT];
uint16_t adc_b_buffer[ADC_READ_COUNT];

uint16_t *get_buffer(uint8_t idx) {
    uint16_t *buf = adc_r_buffer;
    if (idx == 1) {
        buf = adc_g_buffer;
    } else if (idx == 2) {
        buf = adc_b_buffer;
    }

    return buf;
}

volatile uint dma_chan;

// sync primitives
semaphore_t trigger_sem;
queue_t data_queue;

queue_t processing_queue;

// void update_display(LCDAdapter_t *adapter, enum Colors color, uint8_t val) {
//   // 1era linea muestra el valor hex directo
//   uint8_t hex_col = 5;
//   lcd_set_position(adapter, 0, hex_col + color*2);

//   char hex_buffer[3];
//   sprintf(hex_buffer, "%02x", val);

//   lcd_print(adapter, hex_buffer);

//   lcd_set_position(adapter, 1, color*6);

//   int rel_val = val * 100 / 256;
//   char rel_buffer[3];
//   sprintf(rel_buffer, "%c:%d", colorNames[color], rel_val);

//   lcd_print(adapter, rel_buffer);
// }

// inicializa y configura como salida los pines del led
void init_led() {
    uint32_t gpio_mask = 0;
    for (int i = 0; i < 3; i++) {
        gpio_mask |= 1 << led_pins[i];
    }

    gpio_init_mask(gpio_mask);
    gpio_set_dir_out_masked(gpio_mask);
}

void init_adc() {
    // Initalize adc and select current channel
    adc_gpio_init(26 + ADC_CHAN);

    adc_init();

    adc_select_input(ADC_CHAN);

    // adc starts a conversion every clkdiv + 1 clock cycles
    // adc is paced by 48 MHz clock
    // adc should sample every x + 1 cycles
    // for x=47999 -> clkdiv=x+1=48e3 -> sample freq is 1ksps -> fs = 1 kHz
    const int fs = 500;
    const int div = (48000000 / fs) - 1;

    adc_set_clkdiv(div);
    adc_fifo_setup(
        true,  // Write each completed conversion to the sample FIFO
        true,  // Enable DMA data request (DREQ)
        1,     // DREQ (and IRQ) asserted when at least 1 sample present
        false, // We won't see the ERR bit because of 8 bit reads; disable.
        false  // Don't! Shift each sample to 8 bits when pushing to FIFO
    );
}

void print_adc_results(uint16_t *values, int count, int curr_idx) {
    printf("results:\n");

    float sum = 0;

    for (int i = 0; i < count; ++i) {
        uint16_t val = values[i];

        float voltage = val * 3.33f / (1 << 12);

        // corregir respuesta espectral
        // float rel_sensitivity[3] = {.75, .95, .24};
        float rel_sensitivity[3] = {.55, .35, .27};
        // voltage *= rel_sensitivity[curr_idx];

        sum += voltage;

        printf("%04.2f, ", voltage);

        if (i % 10 == 9)
            printf("\n");
    }

    printf("avg: %f\n", sum / count);
}

// void run_sequence(int current_idx, int dma_chan, uint16_t *buf) {
//     int prev_idx = current_idx == 0 ? 2 : current_idx - 1;

//     gpio_put(led_pins[prev_idx], 0);
//     gpio_put(led_pins[current_idx], 1);
//     sleep_ms(500);

//     dma_channel_set_write_addr(dma_chan, buf, true);
//     printf("Starting capture... [%c]\n", colorNames[current_idx]);

//     adc_run(true);
//     dma_channel_wait_for_finish_blocking(dma_chan);

//     printf("capture finished!\n");
//     adc_run(false);
//     adc_fifo_drain();

//     // print_adc_results(buf, ADC_READ_COUNT, current_idx);
// }

float convert_adc_hex(uint8_t idx, uint16_t val) {
    // valores de ADC para sup blanca y negra

    // medidos por el ADC
    float adck[3] = {3046/3, 3200/3, 2421/3};
    // float adcw[3] = {127, 174, 172};

    // medidos a mano
    // float adck[3] = {664, 1660, 897};
    float adcw[3] = {74, 142, 94};

    // limito el valor leido del adc a  los valores de calibracion
    if (val < adcw[idx]) {
        val = adcw[idx];
    } else if (val > adck[idx]) {
        val = adck[idx];
    }

    float relative = (adck[idx] - val) / (adck[idx] - adcw[idx]);
    return relative * 255.f;
}

// necesita saber que color esta procesando
uint8_t process_samples(uint8_t idx) {
    uint16_t *raw_data = get_buffer(idx);

    // aplicar filtro
    uint16_t filtered_data[ADC_READ_COUNT];
    apply_filter(raw_data, filtered_data, ADC_READ_COUNT);

    // de los datos filtrados descartamos los primeros x valores, ya que antes de ese punto la salida no se estabilizo
    const int adc_offset = 70;

    // ADC_READ_COUNT * 2 valores (cada numero tiene siempre 2 caracteres)
    // ADC_READ_COUNT - 1 comas
    //  2 corchetes
    //  1 retorno '\n'
    //  1 null termination '\0'

    char raw_buf[ADC_READ_COUNT * 2 + ADC_READ_COUNT - 1 + 4] = {'['};
    char filtered_buf[ADC_READ_COUNT * 2 + ADC_READ_COUNT - 1 + 4] = {'['};
    int strbuf_offset = 1;

    float sum = 0;
    for (int j = 0; j < ADC_READ_COUNT; ++j) {
        float hex_val_raw = convert_adc_hex(idx, raw_data[j]);

        float hex_val = convert_adc_hex(idx, filtered_data[j]);

        // solo se promedian los valores a partir de adc_offset
        if (j >= adc_offset) {
            sum += hex_val;
        }

        sprintf(raw_buf + strbuf_offset, "%02X", (uint16_t)hex_val_raw);
        strbuf_offset += sprintf(filtered_buf + strbuf_offset, "%02X", (uint16_t)hex_val);
        // Add comma between elements
        if (j < ADC_READ_COUNT - 1) {
            sprintf(raw_buf + strbuf_offset, ",");
            strbuf_offset += sprintf(filtered_buf + strbuf_offset, ",");
        }
    }
    uint8_t avg = sum / (ADC_READ_COUNT - adc_offset);

    sprintf(raw_buf + strbuf_offset, "]\n");
    sprintf(filtered_buf + strbuf_offset, "]\n");
    // envia los valores por puerto serie
    printf("%s %s", raw_buf, filtered_buf);

    return avg;
}

int64_t alarm_cb(alarm_id_t id, void *user_data) {
    adc_run(true);
    return 0;
}

void start_adc_sampling() {
    static int idx = 0;
    // printf("%d\n", idx);

    // cleanup previous run
    if (idx > 0) {
        // printf("stop adc\n");
        adc_run(false);
        // printf("drain fifo\n");
        adc_fifo_drain();

        // printf("prev led off\n");
        gpio_put(led_pins[idx - 1], 0);

        // uint16_t *prev_buffer = adc_r_buffer;
        // if (idx == 2) {
        //     prev_buffer = adc_g_buffer;
        // } else if (idx == 3) {
        //     prev_buffer = adc_g_buffer;
        // }

        uint8_t prev_idx = idx - 1;
        queue_try_add(&processing_queue, &prev_idx);
    }

    // printf("ack interr\n");
    dma_channel_acknowledge_irq0(dma_chan);

    // last iter, shouldn't reset adc
    if (idx == 3) {
        idx = 0;
        return;
    }

    // printf("set curr led\n");
    gpio_put(led_pins[idx], 1);
    // sleep_ms(500);

    uint16_t *buf = get_buffer(idx);

    // printf("restart adc + dma\n");
    dma_channel_set_write_addr(dma_chan, buf, true);
    // defer adc start to alarm
    add_alarm_in_ms(500, alarm_cb, NULL, true);
    idx += 1;

    // printf("end\n");
}

void core1_task();

int main() {
    stdio_init_all();

    sleep_ms(750);

    multicore_launch_core1(core1_task);

    printf("Initializing LED\n");
    init_led();

    printf("Initializing ADC\n");
    init_adc();

    printf("Initializing DMA\n");
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);

    // channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);

    // The channel uses the transfer request signal to pace its data transfer
    // rate. pace the transfer rate based on the DREQ triggered by the ADC FIFO
    // the FIFO was configured to trigger DREQ when 1 sample is present
    channel_config_set_dreq(&cfg, DREQ_ADC);

    dma_channel_configure(dma_chan, &cfg,
                          adc_r_buffer,   // dst
                          &adc_hw->fifo,  // src (the adc FIFO)
                          ADC_READ_COUNT, // number of transfers
                          false           // start inmediately
    );

    // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq1_enabled(dma_chan, true);

    // Configure the processor to run handler when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_1, start_adc_sampling);
    irq_set_enabled(DMA_IRQ_1, true);

    printf("core0 is waiting...\n");
    if (multicore_fifo_pop_blocking() != FLAG) {
        printf("sync mismatch, exiting...\n");
        return 1;
    }
    printf("core1 server is up, core0 starts running...\n");

    queue_init(&processing_queue, sizeof(uint8_t), 3);

    while (1) {
        sem_acquire_blocking(&trigger_sem);

        start_adc_sampling();

        uint32_t measurement = 0;
        for (int i = 0; i < 3; i++) {
            printf("Waiting for samples...\n");

            uint8_t idx;
            queue_remove_blocking(&processing_queue, &idx);

            printf("Processing samples (%c)\n", colorNames[idx]);
            uint8_t result = process_samples(idx);
            printf("Result (avg): %02X\n", result);

            // armo el codigo hexa en base a las mediciones
            measurement |= result << (8 * (2-idx));
        }

        // printf("exit for loop\n");
        queue_try_add(&data_queue, &measurement);
        printf("measure was %06X\n", measurement);

        // for (int i = 0; i < 3; i++) {
        //     uint16_t *buf;
        //     switch (current_idx) {
        //     case R:
        //         buf = adc_r_buffer;
        //         break;
        //     case G:
        //         buf = adc_g_buffer;
        //         break;
        //     default:
        //         buf = adc_b_buffer;
        //     }

        //     run_sequence(i, dma_chan, buf);
        // }
    }

    return 0;
}

void core1_task() {
    if (web_server_init(&data_queue, &trigger_sem)) {
        return;
    }
    // envia un valor arbitrario al nucleo0 indicando que el servidor inicio
    multicore_fifo_push_blocking(FLAG);

    web_server_poll();
}