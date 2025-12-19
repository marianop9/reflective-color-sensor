#include <string.h>

// #include "FreeRTOS.h"
// #include "queue.h"
// #include "stream_buffer.h"

// #include "usbd_cdc_if.h"

#include "cs_usb_comms.h"
#include "cs_usb_comms_internals.h"

static char _cmd_buffer[CMD_BUFFER_MAX_LEN] = {0};
static size_t _cmd_buffer_len = 0;
static size_t _cmd_buffer_prev_len = 0;

/* Clears the buffer and resets all state*/
void cs_clear_cmd_buffer()
{
    memset(_cmd_buffer, 0, CMD_BUFFER_MAX_LEN);
    _cmd_buffer_len = 0;
    _cmd_buffer_prev_len = 0;
}

/* Updates the buffer-related state.
    Must be called everytime the buffer is written to.
*/
void cs_updated_cmd_buffer(size_t bytes_read)
{
    _cmd_buffer_prev_len = _cmd_buffer_len;
    _cmd_buffer_len += bytes_read;
}

/* Returns the current total length of the buffer contents*/
size_t cs_get_cmd_buffer_len()
{
    return _cmd_buffer_len;
}

/* Returns a pointer to the current position in the buffer*/
char *cs_get_free_cmd_buffer()
{
    return (_cmd_buffer + _cmd_buffer_len);
}

/* Returns the remaining bytes left in the buffer, starting from the current position*/
size_t cs_get_free_cmd_buffer_len()
{
    return (CMD_BUFFER_MAX_LEN - _cmd_buffer_len);
}

/* Checks if a command can be found in the buffer.
    If a command is found, `out_cmd` is set to the command ID.
    Returns `true` if a command was found.
*/
bool cs_check_for_command(cs_command *out_cmd)
{
    // check if a command is present in the buffer
    size_t cmd_len = cs_find_cmd();
    bool found_cmd = cmd_len > 0;

    if (found_cmd)
    {
        // parse and set the command
        *out_cmd = cs_parse_cmd();
        // update buffer
        cs_shift_cmd_buffer(cmd_len);
    }

    return found_cmd;
}

/* Attempts to find a command in the buffer.
    A command is any string terminated by a line-feed (`'\n'`)

    If a command is found, returns the length of the found command,
    starting from the start of the buffer
*/
size_t cs_find_cmd()
{
    size_t cmd_len = 0;

    for (size_t i = _cmd_buffer_prev_len; i < _cmd_buffer_len; i++)
    {
        if (_cmd_buffer[i] == '\n')
        {
            // null-terminate command
            _cmd_buffer[i] = '\0';
            cmd_len = i + 1;
            break;
        }
    }

    return cmd_len;
}

/* Attempts to parse a command present in the buffer.
    Returns CMD_ERR if no known command is found.
*/
cs_command cs_parse_cmd()
{
    char *str = _cmd_buffer;
    size_t len = _cmd_buffer_len;

    /* `len` (which represents ALL characters present in the buffer)
        is only specified as a maximum upper bound.

        Since the command in the buffer should be null-terminated, strncmp should
        stop at the first '\0', which may be found in less than `len` characters.

        This function should be called after `cs_find_cmd`, which null-terminates
        the found command.
    */
    cs_command cmd = CS_COMMAND_ERR;

    if (0 == strncmp(str, "PING", len))
    {
        cmd = CS_COMMAND_PING;
    }
    else if (0 == strncmp(str, "TOGGLE_LED", len))
    {
        cmd = CS_COMMAND_TOGGLE_LED;
    }
    else if (0 == strncmp(str, "MEM", len))
    {
        cmd = CS_COMMAND_MEM;
    }
    else if (0 == strncmp(str, "TEST_ADC", len))
    {
        cmd = CS_COMMAND_ADC;
    }
    else if (0 == strncmp(str, "SET_LED", len))
    {
        cmd = CS_COMMAND_LED;
    }

    return cmd;
}

/* Shift the contents of the buffer to the start of the buffer, starting from
    the end of the found command
*/
void cs_shift_cmd_buffer(size_t found_cmd_len)
{
    char *dst = _cmd_buffer;
    char *src = _cmd_buffer + found_cmd_len;
    size_t count = CMD_BUFFER_MAX_LEN - found_cmd_len;

    memmove(dst, src, count);

    // update state
    _cmd_buffer_len = _cmd_buffer_len - found_cmd_len;
}

/* `text` must be a null-terminated C-string */
void cs_build_text_response(cs_response_msg *resp, const char *text)
{
    size_t len = strlen(text);
    if (len > CS_RESPONSE_PAYLOAD_MAX_BYTES)
    {
        len = CS_RESPONSE_PAYLOAD_MAX_BYTES;
    }
    
    resp->id = CS_RESPONSE_TEXT;
    resp->len = len;
    strncpy((char *)resp->payload, text, len);
}

// little-endian formatting
static inline void format_u16_le(uint8_t *dst, uint16_t v)
{
    dst[0] = (uint8_t)(v & 0xFF);
    dst[1] = (uint8_t)(v >> 8);
}

void cs_build_data_response(cs_response_msg *resp, const uint16_t *data, size_t len)
{
    if (len > CS_RESPONSE_PAYLOAD_MAX_BYTES / sizeof(uint16_t))
    {
        len = CS_RESPONSE_PAYLOAD_MAX_BYTES / sizeof(uint16_t);
    }

    resp->id = CS_RESPONSE_U16;
    resp->len = len;
    for (size_t i = 0; i < len/2; i++)
    {
        format_u16_le(&resp->payload[2*i], data[i]);
    }

}