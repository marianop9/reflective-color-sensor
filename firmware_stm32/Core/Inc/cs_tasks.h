#ifndef _CS_TASKS_
#define _CS_TASKS_

#include "FreeRTOS.h"
#include "queue.h"
#include "stream_buffer.h"

// HANDLES ---------------------------------------------------------------------
extern TaskHandle_t task_handle_usb_receiver;
extern TaskHandle_t task_handle_usb_sender;

extern StreamBufferHandle_t sb_usb_recv_buffer;
extern QueueHandle_t q_usb_send_queue;

// STARTUP ---------------------------------------------------------------------
void tasks_init();

// TASKS -----------------------------------------------------------------------
void tasks_usb_receiver(void *arg);
void tasks_usb_sender(void *arg);

#endif // _CS_TASKS_