#ifndef _CS_USB_COMM_
#define _CS_USB_COMM_

#include <stdbool.h>
#include <stdint.h>

#define CS_RESPONSE_PAYLOAD_MAX_BYTES 32

typedef enum
{
    CS_COMMAND_ERR = 0,
    CS_COMMAND_PING,
    CS_COMMAND_MEM,
    CS_COMMAND_TOGGLE_LED, // test onboard led
    CS_COMMAND_ADC,
    CS_COMMAND_SET_LED
} cs_command;

typedef uint8_t cs_response;
enum
{
    CS_RESPONSE_TEXT = 0,
    CS_RESPONSE_U16,
};

typedef struct
{
    cs_response id;
    uint8_t len;
    uint8_t payload[CS_RESPONSE_PAYLOAD_MAX_BYTES];

} cs_response_msg;

void cs_clear_cmd_buffer();

void cs_updated_cmd_buffer(size_t bytes_read);

size_t cs_get_cmd_buffer_len();

char *cs_get_free_cmd_buffer();

size_t cs_get_free_cmd_buffer_len();

uint32_t cs_get_arg(size_t index);

bool cs_check_for_command(cs_command *out_cmd);

void cs_build_text_response(cs_response_msg *resp, const char *text);
void cs_build_data_response(cs_response_msg *resp, const uint16_t *data, size_t len);

#endif // _CS_USB_COMM_