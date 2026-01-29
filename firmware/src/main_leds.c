#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "ws2812.h"

TaskHandle_t main_handle;

void error_blink() {
    for (;;) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(2000));
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void main_task(void *arg) {
    vTaskDelay(pdMS_TO_TICKS(4000));

    int index = 0;
    uint32_t color;
    while (1) {
        color = 0xff << (4 * index);
        bool result = ws2812_set_pixel(0, color);
        if (!result)
            error_blink();

        printf("set %06x\n", color);
        ws2812_sync_pixels();

        vTaskDelay(pdMS_TO_TICKS(2000));
        ws2812_pixels_off();

        vTaskDelay(pdMS_TO_TICKS(2000));
        index += 1;
        if (index == 6)
            index = 0;
    }
}

int main() {
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    ws2812_init(BOARD_WS2812_PIN);

    BaseType_t created =
        xTaskCreate(main_task, "main_task", configMINIMAL_STACK_SIZE, NULL, 10,
                    &main_handle);
    if (created != pdPASS) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    }

    vTaskStartScheduler();
    return 1;
}