#include "FreeRTOS.h"
#include "message_buffer.h"
#include "queue.h"
#include "task.h"

#include "tusb.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "board.h"
#include "drivers/adc.h"
#include "serial_com.h"

#define TASK_DEFAULT_PRIORITY 10

#define TEST_MSG                                                               \
    ("\0\0\0\0\0\0\0\0\0aaaa\x22\xf1xxxxa\0\0\0\x22aaa\xf1xxxx\x22a"           \
     "aaa\xf1xxxx\x22aaaax"                                                    \
     "\xf1xxx\x22aaaa\xf1xxxxaaaaaaaaaa\xf1\0\0xxxxaaaaaaaaaa\xf1xx"           \
     "xxaa"                                                                    \
     "aaaaaaaa\xf1xxxx")

#define TEST_DATA                                                              \
    ((uint16_t[]){0xff22, 0xff22, 0xff22, 0xff22, 0xff22, 0x0011, 0x0011,      \
                  0x0011, 0x0011, 0x0011})

#define ADC_NUM_SAMPLES 40
uint16_t adc_buffer[ADC_NUM_SAMPLES] = {0};

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
QueueHandle_t cmd_queue;

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

/** Arrange a u16 value using little-endian format */
static inline void format_u16_le(uint8_t *dst, uint16_t v) {
    dst[0] = (uint8_t)(v & 0xFF);
    dst[1] = (uint8_t)(v >> 8);
}

/** `out_buf` should be able to hold the resulting formatted message */
size_t format_text_response(uint8_t *out_buf, const char *msg, size_t len) {
    acp_response_t resp;
    if (!acp_build_response(&resp, ACP_RESP_TEXT, (uint8_t *)msg, len)) {
        return 0;
    }

    return acp_format_response(out_buf, &resp);
}

size_t format_u16_data_response(uint8_t *out_buf, const uint16_t *data,
                                size_t len) {
    size_t len_bytes = len * 2;
    // ensure data fits in buffer
    if (len_bytes > ACP_RESP_PAYLOAD_SIZE) {
        return 0;
    }

    acp_response_t resp = {.type = ACP_RESP_DATA,
                           .payload_len_bytes = (uint8_t)len_bytes};

    for (size_t i = 0; i < len; i++) {
        format_u16_le(&resp.payload[i * 2], data[i]);
    }

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
    // assume each response is sent synchronously, so the buffer has at most 1
    // user at a time
    static uint8_t out_buf[ACP_RESP_MAX_SIZE];
    size_t n = 0;

    acp_command_t cmd;
    while (1) {
        xQueueReceive(cmd_queue, &cmd, portMAX_DELAY);

        switch (cmd.type) {
        case ACP_CMD_PING: {
            n = format_text_response(out_buf, "PONG", 4);
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
            n = format_u16_data_response(out_buf, TEST_DATA,
                                         sizeof(TEST_DATA) / 2);
            xMessageBufferSend(tx_msg_buffer, out_buf, n, portMAX_DELAY);
            break;
        }
        case ACP_CMD_ADC:
            // start ADC+DMA
            start_adc_dma();
            // block until it finishes
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            n = format_u16_data_response(out_buf, adc_buffer, ADC_NUM_SAMPLES);
            xMessageBufferSend(tx_msg_buffer, out_buf, n, portMAX_DELAY);
            break;
        case ACP_CMD_SET_LED: {
            // // arg0: led index
            // uint32_t index = cs_get_arg(0);
            // // arg1: 24-bit RGB code
            // uint32_t rgb = cs_get_arg(1);

            // if (led_ctrl_set_buffer2(index, rgb) == 0) {
            //     HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1,
            //                           (uint32_t *)led_ctrl_get_buffer(),
            //                           led_ct`rl_get_buffer_len());
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
            n = format_text_response(out_buf, "unknown cmd", 11);
            xMessageBufferSend(tx_msg_buffer, out_buf, n, portMAX_DELAY);
            break;
        }
    }
}

void adc_finished_cb() {
    // runs in IRQ context
    stop_adc(true);

    BaseType_t xHigherPrioTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(worker_task_handle, &xHigherPrioTaskWoken);

    portYIELD_FROM_ISR(xHigherPrioTaskWoken);
}

int main() {
    /** Peripheral init */
    status_led_init();
    init_adc(BOARD_ADC_CHAN, adc_buffer, ADC_NUM_SAMPLES, adc_finished_cb);

    /** Sync primitives init */
    tx_msg_buffer = xMessageBufferCreate(ACP_RESP_MAX_SIZE + sizeof(size_t));
    cmd_queue = xQueueCreate(2, sizeof(acp_command_t));

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
                          TASK_DEFAULT_PRIORITY, &usb_task_handle);
    assert(created == pdPASS);

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

    acp_command_t cmd;
    bool result = acp_parse_command(&cmd, rx_buf, n);

    if (result) {
        // notify worker
        xQueueSend(cmd_queue, &cmd, 0);
    } else {
        const char error_msg[] = "comando invalido";
        uint8_t out_buf[12 + sizeof(error_msg)];
        size_t n = format_text_response(out_buf, error_msg, strlen(error_msg));
        // shouldn't be any reason for buffer to be full, so don't block
        xMessageBufferSend(tx_msg_buffer, out_buf, n, 0);
    }
}
