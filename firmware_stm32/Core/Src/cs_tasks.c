#include "cs_tasks.h"
#include "cs_usb_comms.h"
#include "cs_led_ctrl.h"
#include "cs_usb.h"

#include "stm32f1xx_hal.h"

#include <stdio.h>

// USB StreamBuffer capacity. Data received via USB is copied into this buffer
#define SB_USB_BUFFER_CAPACITY 64

// FREERTOS HANDLES -------------------------------------------------------------
TaskHandle_t task_handle_usb_receiver;
TaskHandle_t task_handle_usb_sender;

StreamBufferHandle_t sb_usb_recv_buffer;
QueueHandle_t q_usb_send_queue;

// STM/HAL HANDLES -------------------------------------------------------------
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

// STARTUP ---------------------------------------------------------------------
void tasks_init()
{
    led_ctrl_init(htim3.Instance->ARR);

    q_usb_send_queue = xQueueCreate(2, sizeof(cs_response_msg));
    sb_usb_recv_buffer = xStreamBufferCreate(SB_USB_BUFFER_CAPACITY, 1);
}

// TASKS -----------------------------------------------------------------------
void tasks_usb_receiver(void *arg)
{
    size_t available_bytes = 0;
    size_t received_bytes = 0;
    cs_command cmd = CS_COMMAND_ERR;

    for (;;)
    {
        available_bytes = cs_get_free_cmd_buffer_len();
        if (available_bytes == 0)
        {
            /* If the buffer is full (no free bytes), no bytes will be written in the next
            xStreamBufferReceive call. After this, no further updates will be triggered.
            The only way this can happen is if the host sends garbage, or sends multiple commands without ending in a line-feed (\n). The buffer is cleared to avoid this. */
            cs_clear_cmd_buffer();
            available_bytes = cs_get_free_cmd_buffer_len();
        }

        received_bytes = xStreamBufferReceive(sb_usb_recv_buffer,
                                              (void *)cs_get_free_cmd_buffer(),
                                              available_bytes,
                                              portMAX_DELAY);

        if (received_bytes == 0)
        {
            continue;
        }

        cs_updated_cmd_buffer(received_bytes);

        if (!cs_check_for_command(&cmd))
        {
            continue;
        }

        cs_response_msg resp;
        switch (cmd)
        {
        case CS_COMMAND_TOGGLE_LED:
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            cs_build_text_response(&resp, "OK\n");
            break;
        case CS_COMMAND_PING:
            cs_build_text_response(&resp, "PONG\n");
            break;
        case CS_COMMAND_MEM:
        {
            UBaseType_t receiver_stack_free_words = uxTaskGetStackHighWaterMark(NULL);
            UBaseType_t sender_stack_free_words = uxTaskGetStackHighWaterMark(task_handle_usb_sender);

            char buf[32] = {0};
            snprintf(buf,
                     sizeof(buf),
                     "Rcvr:%3lu, Sndr:%3lu\n",
                     receiver_stack_free_words, sender_stack_free_words);
            cs_build_text_response(&resp, buf);
            break;
        }
        case CS_COMMAND_ADC:
        {
            uint16_t buf[16];
            // start ADC+DMA
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)buf, 16);
            // block until it finishes
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            // stop ADC
            HAL_ADC_Stop_DMA(&hadc1);
            // TODO! Error checking with HAL_ADC_GetState?
            cs_build_data_response(&resp, buf, 16);
            break;
        }
        case CS_COMMAND_SET_LED:
        {

            // arg0: led index
            uint32_t index = cs_get_arg(0);
            // arg1: 24-bit RGB code
            uint32_t rgb = cs_get_arg(1);

            if (led_ctrl_set_buffer2(index, rgb) == 0)
            {
                HAL_TIM_PWM_Start_DMA(&htim3,
                                      TIM_CHANNEL_1,
                                      (uint32_t *)led_ctrl_dma_buffer,
                                      LED_CTRL_DMA_BUFFER_LEN);
                // block until it finishes
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                cs_build_text_response(&resp, "OK\n");
            }
            else
            {
                cs_build_text_response(&resp, "failed to set LEDs\n");
            }

            break;
        }
        case CS_COMMAND_ERR:
        default:
            cs_build_text_response(&resp, "unknown cmd\n");
            break;
        }

        xQueueSend(q_usb_send_queue, &resp, portMAX_DELAY);
    }
}

void tasks_usb_sender(void *arg)
{
    for (;;)
    {
        cs_response_msg resp = {0};

        if (pdPASS != xQueueReceive(q_usb_send_queue, &resp, portMAX_DELAY))
        {
            continue;
        }

        usb_send(&resp.id, 1);
        usb_send(&resp.len, 1);
        usb_send(resp.payload, resp.len);
    }
}