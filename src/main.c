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

// ADC pins
#define ADC_CHAN 0

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

// int led_pins[3] = {18, 19, 20};
enum Colors {
  R = 0, G, B
};
char colorNames[] = {'R', 'G', 'B'};

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

int dma_transfer_count = 50;
uint16_t output_buf[50] = {0};

int main() {
  stdio_init_all();

  sleep_ms(1000);
  printf("hello world!\n");
  

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
      
  // Initalize adc and select current channel
  adc_gpio_init(26 + ADC_CHAN);

  adc_init();
  adc_select_input(ADC_CHAN);

  // adc starts a conversion every clkdiv + 1 clock cycles
  // adc is paced by 48 MHz clock
  // adc should sample every 47999 + 1 = 48000 cycles -> sample freq is 1ksps ===> fs = 1 kHz
  adc_set_clkdiv(0);
  adc_fifo_setup(
    true,    // Write each completed conversion to the sample FIFO
    true,    // Enable DMA data request (DREQ)
    1,       // DREQ (and IRQ) asserted when at least 1 sample present
    false,   // We won't see the ERR bit because of 8 bit reads; disable.
    false     // Shift each sample to 8 bits when pushing to FIFO 
  );

  sleep_ms(1000);

  printf("Initial values:\n");
  for (int i = 0; i < 10; ++i) {
    printf("%-3d, ", output_buf[i] & 0x0fff);
  }
  printf("\n\n");

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
    output_buf,   // dst
    &adc_hw->fifo, // src (the adc FIFO)
    dma_transfer_count,  // number of transfers
    true // start inmediately
  );
  
  printf("Starting capture...\n");
  adc_run(true);

  dma_channel_wait_for_finish_blocking(dma_chan);
  printf("capture finished!\n");

  // // once DMA finished stop the adc and clear the FIFO
  adc_run(false);
  adc_fifo_drain();

  
  for (int i = 0; i < dma_transfer_count; ++i) {
    printf("%-3d, ", output_buf[i] & 0x0fff);
    if (i % 10 == 9)
      printf("\n");
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
