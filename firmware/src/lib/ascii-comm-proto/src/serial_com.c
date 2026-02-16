#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial_com.h"

/* -------------------- PRIVATE API -------------------- */

/**
 * @brief List of available commands as strings.
 */
const char *ACP_COMMAND_LIST[] = {
    "PING", "MEM", "SET_RES", "ADC", "SET_LED",
};

const char *ACP_RESPONSE_LIST[] = {
    "ERR",
    "TEXT",
    "DATA",
};

/**
 * @brief Calculates the checksum for a given buffer.
 *
 * This function computes a checksum by XOR-ing all characters in the buffer.
 *
 * @param buffer The buffer for which to calculate the checksum.
 * @return The computed checksum value.
 */
uint8_t calculate_checksum(const char *buffer) {
    if (buffer == NULL) {
        return 0;
    }

    uint8_t checksum = 0;
    while (*buffer) {
        checksum ^= *buffer;
        buffer += 1;
    }
    return checksum;
}

/**
 * @brief Converts a string representation of a command into its corresponding
 * enum value.
 *
 * @param str The (null-terminated) string representation of the command.
 * @return The corresponding acp_type_t enum value. Returns ACP_CMD_INVALID if
 * the string does not match any known command.
 */
acp_command_type_t string_to_command(const char *str) {
    for (acp_command_type_t cmd = 0; cmd < ACP_CMD_COUNT; cmd++) {
        if (strncmp(str, ACP_COMMAND_LIST[cmd], ACP_COMMAND_STR_SIZE) == 0) {
            return cmd;
        }
    }
    return ACP_CMD_INVALID;
}

/**
 * @brief Converts an enum value of command into its string representation.
 *
 * @param type The enum defining the command
 * @return const char* The string representation of the command. Returns NULL if
 * the command is invalid.
 */
const char *command_to_string(acp_command_type_t type) {
    if (type >= 0 && type < ACP_CMD_COUNT) {
        return ACP_COMMAND_LIST[type];
    }

    return NULL;
}

/**
 * @brief Converts an enum value of response type into its string
 * representation.
 *
 * @param type The enum defining the response
 * @return const char* The string representation of the command. Returns NULL if
 * the command is invalid.
 */
const char *response_to_string(acp_response_type_t type) {
    if (type >= 0 && type < ACP_RESP_COUNT) {
        return ACP_RESPONSE_LIST[type];
    }

    return NULL;
}

size_t uint8_to_str(uint8_t num, char *out_buf) {
    if (num == 0) {
        out_buf[0] = '0';
        return 1;
    }

    char buf[3];
    size_t parsed = 0;

    while (num > 0) {
        buf[parsed] = num % 10 + '0';
        parsed += 1;
        num /= 10;
    }

    int j = 0;
    int i = parsed;
    for (; i > 0; j++, i--) {
        out_buf[j] = buf[i - 1];
    }

    return parsed;
}

/* -------------------- PUBLIC API -------------------- */

// bool acp_create_command(char *buffer, acp_command_t cmd) {
//     const char *cmd_str = command_to_string(cmd.type);
//     if (cmd_str == NULL) {
//         return false;
//     }

//     sprintf(buffer, "%c%s,%s,%s%c", ACP_START_CHAR, cmd_str, cmd.payload1,
//             cmd.payload2, ACP_END_CHAR);
//     const uint8_t cs = calculate_checksum(buffer);
//     sprintf(buffer + strlen(buffer), "%02X", cs);

//     return true;
// }

bool acp_parse_command(acp_command_t *cmd, const char *buffer, int length) {
    // attempt to remove any excess characters to make parsing easier.
    if (buffer[length - 1] == '\0') {
        length -= 1;
    }

    if (buffer[length - 1] == '\n') {
        length -= 1;
        if (buffer[length - 1] == '\r') {
            length -= 1;
        }
    }

    if (length < ACP_MINIMUM_DATA_LENGTH) {
        return false;
    }

    char start_char = buffer[0];
    char end_char = buffer[length - 1];
    if (start_char != ACP_START_CHAR || end_char != ACP_END_CHAR) {
        return false;
    }

    memset(cmd, 0, sizeof(acp_command_t));

    char separator[] = {ACP_SEPARATOR_CHAR};
    // First separator (command_string-payload1)
    size_t sep1_index = strcspn(buffer, separator);
    // Second separator (payload1-payload2)
    size_t sep2_index =
        strcspn(buffer + sep1_index + 1, separator) + sep1_index + 1;

    size_t command_str_len = sep1_index - 1;
    if (command_str_len >= ACP_COMMAND_STR_SIZE) {
        return false;
    }

    char command_str[ACP_COMMAND_STR_SIZE];
    memcpy(command_str, buffer + 1, command_str_len);
    command_str[command_str_len] = '\0';
    cmd->type = string_to_command(command_str);

    if (!acp_is_valid_command(cmd)) {
        return false;
    }

    size_t payload1_length = sep2_index - sep1_index - 1;
    if (payload1_length > 0) {
        if (payload1_length >= ACP_PAYLOAD1_SIZE) {
            payload1_length = ACP_PAYLOAD1_SIZE - 1;
        }
        memcpy(cmd->payload1, buffer + sep1_index + 1, payload1_length);
        cmd->payload1[payload1_length] = '\0';
    }

    size_t payload2_length = length - sep2_index - 2;
    if (payload2_length > 0) {
        if (payload2_length >= ACP_PAYLOAD2_SIZE) {
            payload2_length = ACP_PAYLOAD2_SIZE - 1;
        }
        memcpy(cmd->payload2, buffer + sep2_index + 1, payload2_length);
        cmd->payload2[payload2_length] = '\0';
    }

    return true;
}

bool acp_is_valid_command(const acp_command_t *cmd) {
    return cmd->type != ACP_CMD_INVALID;
}

bool acp_build_response(acp_response_t *resp, acp_response_type_t type,
                        const uint8_t *payload, uint8_t payload_len_bytes) {

    if (payload_len_bytes > ACP_RESP_PAYLOAD_SIZE) {
        return false;
    }

    resp->type = type;
    resp->payload_len_bytes = payload_len_bytes;
    memcpy(resp->payload, payload, payload_len_bytes);

    return true;
}

size_t acp_format_response(uint8_t *buffer, acp_response_t *resp) {
    // could add checks to ensure buffer is big enough
    const char *resp_str = response_to_string(resp->type);
    if (resp_str == NULL) {
        return 0;
    }

    size_t n = 0;
    const size_t resp_strn_len = strlen(resp_str);

    // start char
    buffer[n] = ACP_START_CHAR;
    n += 1;
    // response_type (text)
    memcpy(buffer + n, resp_str, resp_strn_len);
    n += resp_strn_len;
    // separator char
    buffer[n] = ACP_SEPARATOR_CHAR;
    n += 1;
    // response size (text)
    n += uint8_to_str(resp->payload_len_bytes, buffer + n);
    // payload separator char
    buffer[n] = ACP_PAYLOAD_START_CHAR;
    n += 1;
    // response payload (raw bytes)
    memcpy(buffer + n, resp->payload, resp->payload_len_bytes);
    n += resp->payload_len_bytes;

    // // end char
    // buffer[n] = ACP_END_CHAR;
    // n += 1;

    return n;
}

/*
&LED,0xff00000,*0xff


"Err: numero invalido"
char == uint8_t

[0x12,0xf2,0xf2,0x033]

TEXT
DATA

&TEXT,20,Err: numero invalido*
&DATA,4,0x12,0xf2,0xf2,0x033*

*/