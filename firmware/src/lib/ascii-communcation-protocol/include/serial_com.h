/** Code based on the "ascii-communication-protocol" library by "esadigov".
 * 
 * https://github.com/esadigov/ascii-communication-protocol
 */

#ifndef ACP_SERIAL_COM_H
#define ACP_SERIAL_COM_H

#include <stdbool.h>
#include <stdint.h>

// Constants for serial communication protocol
/**< Character that signifies the start of a command.   */
#define ACP_START_CHAR '&'
/**< Character that signifies the end of a command. */
#define ACP_END_CHAR '*'
/**< Character used to separate commands and payloads. */
#define ACP_CMD_SEPARATOR ','

// Sizes and definitions
/** Command buffer size. */
#define ACP_CMD_BUFFER_SIZE 128
/** Minimum length of command buffer. */
#define ACP_MINIMUM_DATA_LENGTH 8
/** Number of components expected in a parsed command. */
#define ACP_CMD_COMPONENTS 4
/** Maximum size for the command string. */
#define ACP_COMMAND_STR_SIZE 10
/** Maximum size for the first payload. */
#define ACP_PAYLOAD1_SIZE 10
/** Maximum size for the second payload. */
#define ACP_PAYLOAD2_SIZE 10
/** Maximum size for the checksum string. */
#define ACP_CHECKSUM_STR_SIZE 10

/**
 * @brief List of available commands as strings.
 */
const char *ACP_COMMAND_LIST[] = {"COMMAND1", "COMMAND2"};

/**
 * @brief Enum defining the available commands.
 * 
 * First command must have value `0`.
 */
typedef enum {
    COMMAND1 = 0,     /**< Command 1. */
    COMMAND2,     /**< Command 2. */
    NUM_COMMANDS, /**< Total number of commands. */
    INVALID_CMD   /**< Invalid command. */
} acp_command_type_t;

/**
 * @brief Structure representing a serial communication command.
 */
typedef struct {
    acp_command_type_t cmd;           /**< The command type. */
    char payload1[ACP_PAYLOAD1_SIZE]; /**< The first payload data. */
    char payload2[ACP_PAYLOAD2_SIZE]; /**< The second payload data. */
    uint8_t checksum; /**< Checksum for data integrity verification. */
} acp_command_t;

/**
 * @brief Creates a command string for serial communication from a acp_command_t
 structure.
 *
 * @param buffer The buffer to store the formatted command string.
 * @param cmd The `acp_command_t` structure value.
 * @return true if creation was successful, false otherwire.
 */
bool acp_create_command(char *buffer, acp_command_t cmd);

/**
 * @brief Parses a command string and fills a acp_command_t structure.
 *
 * This function extracts the command, payloads, and checksum from a command
 * string and stores them in a acp_command_t structure.
 *
 * @param cmd Pointer to the acp_command_t structure to be filled with parsed
 * data.
 * @param buffer The command string to parse.
 * @param length The length of the command string.
 * @return true if parsing was successful, false otherwise.
 */
bool acp_parse_command(acp_command_t *cmd, const char *buffer, int length);

/**
 * @brief Checks if a given command structure contains a valid command.
 *
 * @param cmd Pointer to the acp_command_t structure to check.
 * @return true if the command is valid, false otherwise.
 */
bool acp_is_valid_command(const acp_command_t *cmd);

#endif // ACP_SERIAL_COM_H
