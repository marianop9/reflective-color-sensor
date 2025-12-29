#ifndef _CS_USB_
#define _CS_USB_

#include <stdint.h>

void usb_recv_ISR(uint8_t *buf, uint32_t len);
void usb_send(uint8_t *buf, size_t len);

#endif // _CS_USB_