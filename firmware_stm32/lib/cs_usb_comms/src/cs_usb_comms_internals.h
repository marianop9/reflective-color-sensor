#include "cs_usb_comms.h"

#define CMD_BUFFER_MAX_LEN 32

size_t cs_find_cmd();
cs_command cs_parse_cmd();
int cs_parse_args(char *args_start);
void cs_shift_cmd_buffer(size_t found_cmd_len);