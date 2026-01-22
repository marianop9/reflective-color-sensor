#include "FreeRTOS.h"
#include "message_buffer.h"
#include "queue.h"
#include "task.h"

#include "tusb.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "serial_com.h"

#define TASK_DEFAULT_PRIORITY 10

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

TaskHandle_t usb_task_handle = NULL;
TaskHandle_t worker_task_handle = NULL;
MessageBufferHandle_t tx_msg_buffer;

acp_command_t current_cmd;

void status_led_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

void status_led_set(bool state) { gpio_put(PICO_DEFAULT_LED_PIN, state); }

void led_task(void *arg) {
    bool led_state = false;

    while (1) {
        vTaskDelay(blink_interval_ms / portTICK_PERIOD_MS);

        if (blink_enable) {
            status_led_set(led_state);
            led_state = 1 - led_state; // toggle
        }
    }
}

/** `out_buf` should be able to hold the resulting formatted message */
size_t format_text_response(uint8_t *out_buf, const char *msg, size_t len) {
    acp_response_t resp;
    acp_build_response(&resp, ACP_RESP_TEXT, len, (uint8_t *)msg);

    return acp_format_response(out_buf, &resp);
}

void usb_task(void *arg) {
    // avoid using up the task's stack
    static uint8_t tx_buf[ACP_RESP_MAX_SIZE];
    size_t tx_buf_len = 0;
    size_t tx_buf_idx = 0;

    while (1) {
        tud_task();

        if (tx_buf_len == 0) {
            // if no message is currently in buffer, attempt to get a new one
            size_t n = xMessageBufferReceive(tx_msg_buffer, tx_buf,
                                             sizeof(tx_buf), pdMS_TO_TICKS(3));

            if (n > 0) {
                tx_buf_len = n;
                tx_buf_idx = 0;
            }
        }

        if (tx_buf_len > 0 && tud_cdc_connected()) {
            // if a message is currently in buffer, ensure it's completely sent
            size_t available = tud_cdc_write_available();
            if (available == 0)
                continue;

            size_t remaining = tx_buf_len - tx_buf_idx;
            size_t chunk = remaining > available ? available : remaining;

            tx_buf_idx += tud_cdc_write(tx_buf + tx_buf_idx, chunk);
            if (tx_buf_idx >= tx_buf_len) {
                // the whole buffer was sent, flush and reset state
                tx_buf_idx = 0;
                tx_buf_len = 0;
                tud_cdc_write_flush();
            }
        }
    }
}

void worker_task(void *arg) {

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        switch (current_cmd.type) {
        case ACP_CMD_PING: {
            uint8_t out_buf[16];
            size_t n = format_text_response(out_buf, "PONG", 4);
            xMessageBufferSend(tx_msg_buffer, out_buf, n, portMAX_DELAY);
            break;
        }
        case ACP_CMD_MEM: {
            // UBaseType_t receiver_stack_free_words =
            //     uxTaskGetStackHighWaterMark(NULL);
            // UBaseType_t sender_stack_free_words =
            //     uxTaskGetStackHighWaterMark(task_handle_usb_sender);

            // char buf[32] = {0};
            // snprintf(buf, sizeof(buf), "Rcvr:%3lu, Sndr:%3lu\n",
            //          receiver_stack_free_words, sender_stack_free_words);
            // cs_build_text_response(&resp, buf);
            // break;
        }
        case ACP_CMD_ADC: {
            // uint16_t buf[16];
            // // start ADC+DMA
            // HAL_ADC_Start_DMA(&hadc1, (uint32_t *)buf, 16);
            // // block until it finishes
            // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            // // stop ADC
            // HAL_ADC_Stop_DMA(&hadc1);
            // // TODO! Error checking with HAL_ADC_GetState?
            // cs_build_data_response(&resp, buf, 16);
            // break;
        }
        case ACP_CMD_SET_LED: {
            // // arg0: led index
            // uint32_t index = cs_get_arg(0);
            // // arg1: 24-bit RGB code
            // uint32_t rgb = cs_get_arg(1);

            // if (led_ctrl_set_buffer2(index, rgb) == 0) {
            //     HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1,
            //                           (uint32_t *)led_ctrl_get_buffer(),
            //                           led_ctrl_get_buffer_len());
            //     // block until it finishes
            //     ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            //     cs_build_text_response(&resp, "OK\n");
            // } else {
            //     cs_build_text_response(&resp, "failed to set LEDs\n");
            // }

            // break;
        }
        case ACP_CMD_INVALID:
        default:
            uint8_t out_buf[24];
            size_t n = format_text_response(out_buf, "unknown cmd", 11);
            xMessageBufferSend(tx_msg_buffer, out_buf, n, portMAX_DELAY);
            break;
        }
    }
}

int main() {
    /** Peripheral init */
    status_led_init();

    /** Sync primitives init */
    tx_msg_buffer = xMessageBufferCreate(ACP_RESP_MAX_SIZE + sizeof(size_t));

    // tusb_rhport_init_t dev_init = {
    //     .role = TUSB_ROLE_DEVICE,
    //     .speed = TUSB_SPEED_FULL,
    // };
    tusb_init();

    /** RTOS tasks */
    BaseType_t created =
        xTaskCreate(led_task, "led_task", configMINIMAL_STACK_SIZE, NULL,
                    TASK_DEFAULT_PRIORITY - 1, NULL);
    assert(created == pdPASS);

    created = xTaskCreate(worker_task, "worker_task", configMINIMAL_STACK_SIZE,
                          NULL, TASK_DEFAULT_PRIORITY, &worker_task_handle);
    assert(created == pdPASS);

    created = xTaskCreate(usb_task, "usb_task", configMINIMAL_STACK_SIZE, NULL,
                          TASK_DEFAULT_PRIORITY + 1, &usb_task_handle);
    assert(created == pdPASS);

    // BaseType_t created_worker = xTaskCreate(
    // worker_task, "worker", configMINIMAL_STACK_SIZE, NULL, 10, NULL);

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
// Within 7ms, device must draw an average of current less than 2.5 mA from
// bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

void tud_cdc_rx_cb(uint8_t itf) {
    uint8_t rx_buf[ACP_CMD_MAX_SIZE + 1];

    // for now we expect to receive the whole command at once
    uint32_t n = tud_cdc_read(rx_buf, ACP_CMD_MAX_SIZE);

    if (n == 0)
        return;

    bool result = acp_parse_command(&current_cmd, rx_buf, n);

    if (result) {
        // notify worker
        xTaskNotifyGive(worker_task_handle);
    } else {
        const char error_msg[] = "comando invalido";
        uint8_t out_buf[12 + sizeof(error_msg)];
        size_t n = format_text_response(out_buf, error_msg, strlen(error_msg));
        // shouldn't be any reason for buffer to be full, so don't block
        xMessageBufferSend(tx_msg_buffer, out_buf, n, 0);
    }
}
