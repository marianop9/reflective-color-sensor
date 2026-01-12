#include "FreeRTOS.h"
#include "task.h"

#include "hardware/gpio.h"

#define PANIC_LED PICO_DEFAULT_LED_PIN

void panic_blink() {
    gpio_init(PANIC_LED);
    gpio_set_dir(PANIC_LED, GPIO_OUT);
    for (;;) {
        gpio_put(PANIC_LED, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(PANIC_LED, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vApplicationMallocFailedHook(void) {
    __breakpoint();
    taskDISABLE_INTERRUPTS();
    panic_blink();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;

    __breakpoint();
    taskDISABLE_INTERRUPTS();
    panic_blink();
}