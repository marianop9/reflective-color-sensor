#include "usbd_cdc_if.h"

#include "cs_tasks.h"

void usb_recv_ISR(uint8_t *buf, uint32_t len)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    xStreamBufferSendFromISR(sb_usb_recv_buffer,
                             buf,
                             len,
                             &higherPriorityTaskWoken);

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void usb_send(uint8_t *buf, size_t len)
{
    uint8_t state;

    do
    {
        state = CDC_Transmit_FS(buf, len);
        if (state == USBD_BUSY)
        {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    } while (state == USBD_BUSY);
}