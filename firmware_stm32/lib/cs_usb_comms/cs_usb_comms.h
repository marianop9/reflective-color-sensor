#ifndef _CS_USB_COMM_
#define _CS_USB_COMM_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    CMD_ERR = 0,
    CMD_PING,
    CMD_TOGGLE_LED,
} cs_command_id;

typedef enum
{
    RESP_ERR = 0,
    RESP_TEXT,
} cs_response_id;

typedef union
{
    char text[16]; // simple text responses: OK, PONG, etc
} cs_response_payload;

typedef struct
{
    cs_response_id id;
    cs_response_payload payload;

} cs_response_msg;

void cs_clear_cmd_buffer();

void cs_updated_cmd_buffer(size_t bytes_read);

size_t cs_get_cmd_buffer_len();

char *cs_get_remaining_cmd_buffer();

size_t cs_get_remaining_cmd_buffer_len();

bool cs_check_for_command(cs_command_id *out_cmd);

// must be called from CDC_Receive_FS
// void cs_usb_recv_ISR(uint8_t *buf, uint32_t len);

// BaseType_t cs_usb_send_blocking(cs_response_msg *msg);

#endif // _CS_USB_COMM_