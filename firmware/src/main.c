#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usbproto.h"

/** Programa basico de prueba 
 * 
 * 
*/

void led_task(void *arg) {
    bool state = 0;
    while (1) {
        state = !state;
        gpio_put(PICO_DEFAULT_LED_PIN, state);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void usb_task(void *arg) {
    usb_proto_init();

    while (1) {
        tud_task();
        process_commands();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

int main() {
    
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    
    BaseType_t created = xTaskCreate(led_task, "led_task", configMINIMAL_STACK_SIZE, NULL, 10, NULL);
    BaseType_t created_usb = xTaskCreate(usb_task, "usb_task", configMINIMAL_STACK_SIZE, NULL, 11, NULL);
    
    
    if (created != pdPASS || created_usb != pdPASS) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    }

    vTaskStartScheduler();
    return 1;
}
