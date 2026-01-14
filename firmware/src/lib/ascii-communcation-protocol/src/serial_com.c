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
    "PING", "MEM", "TOGGLE_LED", "ADC", "SET_LED",
};

const char *ACP_RESPONSE_LIST[] = {
    "ACK",
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
 * @param str The string representation of the command.
 * @return The corresponding acp_type_t enum value. Returns ACP_CMD_INVALID if
 * the string does not match any known command.
 */
acp_command_type_t string_to_command(const char *str) {
    for (acp_command_type_t cmd = 0; cmd < ACP_CMD_COUNT; cmd++) {
        if (strcmp(str, ACP_COMMAND_LIST[cmd]) == 0) {
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
    if (buffer[length - 1] == '\n' || buffer[length - 1] == '\r') {
        length--;
    }

    if (length < ACP_MINIMUM_DATA_LENGTH || buffer[0] != ACP_START_CHAR ||
        buffer[length - 3] != ACP_END_CHAR) {
        return false;
    }

    memset(cmd, 0, sizeof(acp_command_t));

    char command_str[ACP_COMMAND_STR_SIZE];
    char checksum[ACP_CHECKSUM_STR_SIZE];

    /* the length of all parts of the command is hardcoded, rendering most
        ACP_*_SIZE defines useless (e.g., ACP_COMMAND_STR_SIZE)

        this will also fail if no payloads are provided. 
    */
    int parsed = sscanf(buffer, "&%10[^,],%10[^,],%10[^*]*%2[^*]", command_str,
                        cmd->payload1, cmd->payload2, checksum);

    cmd->type = string_to_command(command_str);

    if (parsed != ACP_CMD_COMPONENTS || !acp_is_valid_command(cmd))
        return false;

    cmd->checksum = strtol(checksum, NULL, 16);

    char temp[256];
    if (length - 2 >= (int)sizeof(temp)) {
        return false;
    }

    strncpy(temp, buffer, length - 2);
    temp[length - 2] = '\0';

    if (calculate_checksum(temp) != cmd->checksum) {
        return false;
    }

    return true;
}

bool acp_is_valid_command(const acp_command_t *cmd) {
    return cmd->type != ACP_CMD_INVALID;
}

bool acp_create_response(uint8_t *buffer, acp_response_t *resp) {
    // could add checks to ensure buffer is big enough
    const char *resp_str = response_to_string(resp->type);
    if (resp_str == NULL)
        return false;

    int n = 0;
    const size_t resp_strn_len = strlen(resp_str);

    buffer[n] = ACP_START_CHAR;
    n += 1;

    memcpy(buffer + n, resp_str, resp_strn_len);
    n += resp_strn_len;

    buffer[n] = ',';
    n += 1;

    buffer[n] = resp->len_bytes;
    n += 1;

    buffer[n] = ',';
    n += 1;

    memcpy(buffer + n, resp->payload, resp->len_bytes);
    n += resp->len_bytes;

    buffer[n] = ACP_END_CHAR;

    return true;
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