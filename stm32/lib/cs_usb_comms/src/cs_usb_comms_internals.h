#include "cs_usb_comms.h"

#define CMD_BUFFER_MAX_LEN 32

size_t cs_find_cmd();
cs_command_id cs_parse_cmd();
void cs_shift_cmd_buffer(size_t found_cmd_len);