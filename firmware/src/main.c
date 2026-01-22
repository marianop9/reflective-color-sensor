#include "FreeRTOS.h"
#include "queue.h"
#include "stream_buffer.h"
#include "task.h"

#include "tusb.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "serial_com.h"

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static bool blink_enable = false;
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static uint8_t tx_buf[ACP_RESP_MAX_SIZE] = {0};
static uint32_t tx_buf_len = 0;

const char msg[] = "hola sapeeeee\n";

TaskHandle_t usb_task_handle = NULL;

void status_led_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

void status_led_set(bool state) { gpio_put(PICO_DEFAULT_LED_PIN, state); }

void led_task(void *arg) {
    static bool led_state = false;

    while (1) {
        vTaskDelay(blink_interval_ms / portTICK_PERIOD_MS);

        if (blink_enable) {
            status_led_set(led_state);
            led_state = 1 - led_state; // toggle
        }

        // tud_cdc_write(msg, strlen(msg));
    }
}

void usb_task(void *arg) {
    uint32_t tx_buf_index = 0;

    while (1) {
        tud_task();

        if (tx_buf_index > 0 || ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3))) {

            if (tx_buf_index >= tx_buf_len) {
                // the whole buffer was sent, reset state
                tx_buf_index = 0;
                tx_buf_len = 0;
            } else {
                tx_buf_index += tud_cdc_write(tx_buf + tx_buf_index,
                                              tx_buf_len - tx_buf_index);
                tud_cdc_write_flush();
            }
        }
    }
}

// void worker_task(void *arg) {}

int main() {
    /** Peripheral init */
    status_led_init();

    /** Sync primitives init */

    // tusb_rhport_init_t dev_init = {
    //     .role = TUSB_ROLE_DEVICE,
    //     .speed = TUSB_SPEED_FULL,
    // };
    tusb_init();

    /** RTOS tasks */
    BaseType_t created = xTaskCreate(led_task, "led_task",
                                     configMINIMAL_STACK_SIZE, NULL, 10, NULL);
    BaseType_t created_usb =
        xTaskCreate(usb_task, "usb_task", configMINIMAL_STACK_SIZE, NULL, 11,
                    &usb_task_handle);

    // BaseType_t created_worker = xTaskCreate(
    // worker_task, "worker", configMINIMAL_STACK_SIZE, NULL, 10, NULL);

    assert(created == pdPASS && created_usb == pdPASS);
    blink_enable = true;

    vTaskStartScheduler();
    return 1;
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void)itf;
    (void)rts;

    // TODO set some indicator
    if (dtr) {
        // Terminal connected
        blink_enable = true;
    } else {
        // Terminal disconnected
        blink_enable = false;
    }
}

// Invoked when device is mounted
void tud_mount_cb(void) { blink_interval_ms = BLINK_MOUNTED; }

// Invoked when device is unmounted
void tud_umount_cb(void) { blink_interval_ms = BLINK_NOT_MOUNTED; }

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

void tud_cdc_rx_cb(uint8_t itf) {
    uint8_t rx_buf[ACP_RESP_PAYLOAD_SIZE];

    uint32_t n = tud_cdc_read(rx_buf, ACP_RESP_PAYLOAD_SIZE);
    acp_response_t resp;

    if (n > 0 && acp_build_response(&resp, ACP_RESP_TEXT, n, rx_buf)) {
        tx_buf_len = acp_format_response(tx_buf, &resp);
        xTaskNotifyGive(usb_task_handle);
    }
}