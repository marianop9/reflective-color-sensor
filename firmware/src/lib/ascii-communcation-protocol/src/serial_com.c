#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial_com.h"

/* -------------------- PRIVATE API -------------------- */

/**
 * @brief Calculates the checksum for a given buffer.
 *
 * This function computes a checksum by XOR-ing all characters in the buffer.
 *
 * @param buffer The buffer for which to calculate the checksum.
 * @return The computed checksum value.
 */
uint8_t acp_calculate_checksum(const char *buffer) {
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
 * @return The corresponding acp_type_t enum value. Returns INVALID_CMD if the
 * string does not match any known command.
 */
acp_command_type_t acp_string_to_command(const char *str) {
    for (acp_command_type_t cmd = 0; cmd < NUM_COMMANDS; cmd++) {
        if (strcmp(str, ACP_COMMAND_LIST[cmd]) == 0) {
            return cmd;
        }
    }
    return INVALID_CMD;
}

/**
 * @brief Converts an enum value of command into its string representation.
 *
 * @param cmd The enum defining the command
 * @return const char* The string representation of the command. Returns NULL if
 * the command is invalid.
 */
const char *acp_command_to_string(acp_command_type_t cmd) {
    if (cmd >= 0 && cmd < NUM_COMMANDS) {
        return ACP_COMMAND_LIST[cmd];
    }

    return NULL;
}

/* -------------------- PUBLIC API -------------------- */

bool acp_create_command(char *buffer, acp_command_t cmd) {
    const char *cmd_str = acp_command_to_string(cmd.cmd);
    if (cmd_str == NULL) {
        return false;
    }

    sprintf(buffer, "%c%s,%s,%s%c", ACP_START_CHAR, cmd_str, cmd.payload1,
            cmd.payload2, ACP_END_CHAR);
    const uint8_t cs = acp_calculate_checksum(buffer);
    sprintf(buffer + strlen(buffer), "%02X", cs);

    return true;
}

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

    int parsed = sscanf(buffer, "&%10[^,],%10[^,],%32[^*]*%2[^*]", command_str,
                        cmd->payload1, cmd->payload2, checksum);

    cmd->cmd = acp_string_to_command(command_str);

    if (parsed != ACP_CMD_COMPONENTS || !acp_is_valid_command(cmd))
        return false;

    cmd->checksum = strtol(checksum, NULL, 16);

    char temp[256];
    if (length - 2 >= (int)sizeof(temp)) {
        return false;
    }

    strncpy(temp, buffer, length - 2);
    temp[length - 2] = '\0';

    if (acp_calculate_checksum(temp) != cmd->checksum) {
        return false;
    }

    return true;
}

bool acp_is_valid_command(const acp_command_t *cmd) {
    return cmd->cmd != INVALID_CMD;
}
