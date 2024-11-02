#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

#include "lcd.h"

/*
// LCD pins
#define LCD_PIN_RS 16
#define LCD_PIN_RW 17
#define LCD_PIN_ENABLE 18

#define LCD_PIN_D7 19
#define LCD_PIN_D6 20
#define LCD_PIN_D5 21
#define LCD_PIN_D4 22 
*/

#define ADC_CHAN 0

#define LED_PIN_R 18 // 630 nm
#define LED_PIN_G 19 // 520 nm
#define LED_PIN_B 20 // 470 nm

void configure_pin(uint32_t pin, bool out) {
  gpio_init(pin);
  gpio_set_dir(pin, out);
}

void set_pin(uint32_t pin, bool value) {
  gpio_put(pin, value);
}

bool get_pin(uint32_t pin) {
  return gpio_get(pin);
}

int led_pins[3] = {LED_PIN_R, LED_PIN_G, LED_PIN_B};
enum Colors {
  R = 0, G, B
};
char colorNames[] = {'R', 'G', 'B'};

#define ADC_READ_COUNT 50
uint16_t adc_r_buffer[ADC_READ_COUNT] = {0};
uint16_t adc_g_buffer[ADC_READ_COUNT] = {0};
uint16_t adc_b_buffer[ADC_READ_COUNT] = {0};

void update_display(LCDAdapter_t *adapter, enum Colors color, uint8_t val) {
  // 1era linea muestra el valor hex directo
  uint8_t hex_col = 5;
  lcd_set_position(adapter, 0, hex_col + color*2);

  char hex_buffer[3];
  sprintf(hex_buffer, "%02x", val);

  lcd_print(adapter, hex_buffer);

  lcd_set_position(adapter, 1, color*6);

  int rel_val = val * 100 / 256;
  char rel_buffer[3];
  sprintf(rel_buffer, "%c:%d", colorNames[color], rel_val);

  lcd_print(adapter, rel_buffer); 
}

// inicializa y configura como salida los pines del led
void init_led() {
  uint32_t gpio_mask = 0;
  for (int i=0; i<3; i++) {
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
  // adc should sample every 47999 + 1 = 48000 cycles -> sample freq is 1ksps ===> fs = 1 kHz
  adc_set_clkdiv(47999);
  adc_fifo_setup(
    true,    // Write each completed conversion to the sample FIFO
    true,    // Enable DMA data request (DREQ)
    1,       // DREQ (and IRQ) asserted when at least 1 sample present
    false,   // We won't see the ERR bit because of 8 bit reads; disable.
    false     // Don't! Shift each sample to 8 bits when pushing to FIFO 
  );
}

void print_adc_results(uint16_t* values, int count, int curr_idx) {
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

  printf("avg: %f\n", sum/count);
}

void run_sequence(int current_idx, int dma_chan) {
  int prev_idx = current_idx == 0 
    ? 2
    : current_idx - 1;

  gpio_put(led_pins[prev_idx], 0);
  gpio_put(led_pins[current_idx], 1);
  sleep_ms(1000);

  uint16_t* buf;
  switch (current_idx) {
    case R:
      buf = adc_r_buffer;
      break;
    case G:
      buf = adc_g_buffer;
      break;
    default:
      buf = adc_b_buffer;
  }

  dma_channel_set_write_addr(dma_chan, buf, true);
  printf("Starting capture... [%c]\n", colorNames[current_idx]);

  adc_run(true);
  dma_channel_wait_for_finish_blocking(dma_chan);

  printf("capture finished!\n");
  adc_run(false);
  adc_fifo_drain();

  print_adc_results(buf, ADC_READ_COUNT, current_idx);
}

int main() {
  stdio_init_all();

  sleep_ms(5000);
  printf("hello world!\n");
  
  init_led();
  
  // adc test
  // LCDAdapter_t* adapter = lcd_create(
  //     configure_pin, set_pin, get_pin, sleep_us,
  //     LCD_PIN_RS, LCD_PIN_RW, LCD_PIN_ENABLE,
  //     LCD_PIN_D7,  LCD_PIN_D6,  LCD_PIN_D5,  LCD_PIN_D4
  // );
  // lcd_init(adapter);
  // update_display(adapter, R, 255);
  // update_display(adapter, G, 0);
  // update_display(adapter, B, 64);
      
  init_adc();
  sleep_ms(1000);

  // printf("Initial values:\n");
  // for (int i = 0; i < 10; ++i) {
  //   printf("%-3d, ", output_buf[i] & 0x0fff);
  // }
  // printf("\n\n");

  uint dma_chan = dma_claim_unused_channel(true);
  dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
  
  // channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
  channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
  channel_config_set_read_increment(&cfg, false);
  channel_config_set_write_increment(&cfg, true);

  // The channel uses the transfer request signal to pace its data transfer rate.
  // pace the transfer rate based on the DREQ triggered by the ADC FIFO
  // the FIFO was configured to trigger DREQ when 1 sample is present 
  channel_config_set_dreq(&cfg, DREQ_ADC);

  dma_channel_configure(
    dma_chan, 
    &cfg, 
    adc_r_buffer,   // dst
    &adc_hw->fifo, // src (the adc FIFO)
    ADC_READ_COUNT,  // number of transfers
    false   // start inmediately
  );
  
  int current_idx = 0;
  while (1) {
    run_sequence(current_idx, dma_chan);

    current_idx++;
    if (current_idx > 2) {
      sleep_ms(5000);
      current_idx = 0;
      printf("\n\n");
    }
  }
  
  // uint gpio_mask = 1 << led_pins[0] | 1 << led_pins[1] | 1 << led_pins[2];
  // gpio_init_mask(gpio_mask);
  // gpio_set_dir_out_masked(gpio_mask);

  // while (1) {
  //   // for (int i=0; i<3; i++) {
  //   //     gpio_put(led_pins[i], 1);
  //   //     sleep_ms(500);
  //   //     gpio_put(led_pins[i], 0);
  //   // }
  //   printf("led on\n");
  //   cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
  //   sleep_ms(500);
  //   printf("led off\n");
  //   cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
  //   sleep_ms(500);    
  // }
  while(1) {}

  return 0;
}
