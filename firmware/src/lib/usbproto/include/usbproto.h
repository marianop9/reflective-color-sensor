#ifndef USBPROTO_H_
#define USBPROTO_H_

#include <stdint.h>
#include <stdbool.h>
#include "tusb.h"
#include "serial_com.h"

#define MAX_COMMAND_LEN 32

static uint8_t rx_buffer[MAX_COMMAND_LEN];


void usb_proto_init(void);
void sendData(void);
void process_commands(void);
#endif /* _USBPROTO_H_ */
