/** Code based on the "ascii-communication-protocol" library by "esadigov".
 *
 * https://github.com/esadigov/ascii-communication-protocol
 */

/** Expected formats:
 *
 * COMMAND
 * (1) ACP_START_CHAR           (&)
 * (2) CMD_STR                  req. max 10 chars
 * (3) ACP_SEPARATOR_CHAR       (,)
 * (4) PAYLOAD_1                opt. max 10 chars
 * (5) ACP_SEPARATOR_CHAR       (,)
 * (6) PAYLOAD_2                opt. max 10 chars
 * (7) ACP_END_CHAR             (*)
 * (8) CHECKSUM                 opt. 8-bit checksum --- IGNORE!
 *
 * RESPONSE
 * Actually not ASCII-encoded, but easier to work with.
 * (1) ACP_START_CHAR           (&)
 * (2) RESP_STR                 req. ASCII text
 * (3) ACP_SEPARATOR_CHAR       (,)
 * (4) PAYLOAD_LENGTH_BYTES     req. ASCII nums (length specified in BYTES)
 * (5) ACP_PAYLOAD_START_CHAR   (;)
 * (6) PAYLOAD                  req. raw data. length is PAYLOAD_LENGTH bytes.
 * (7) ACP_END_CHAR             (*)
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
/**< Character used to separate ASCII commands/arguments. */
#define ACP_SEPARATOR_CHAR ','
/**< Character used to mark the start of the (potentially binary) payload. */
#define ACP_PAYLOAD_START_CHAR ';'

// Sizes and definitions
/** Minimum length of command buffer. */
#define ACP_MINIMUM_DATA_LENGTH 5
/** Number of components expected in a parsed command. */
#define ACP_CMD_COMPONENTS 4
/** Maximum size for the command string. */
#define ACP_COMMAND_STR_SIZE 10
/** Maximum size for the first payload. */
#define ACP_PAYLOAD1_SIZE 10
/** Maximum size for the second payload. */
#define ACP_PAYLOAD2_SIZE 10
/** Command buffer size. (A command has 4 control characters) */
#define ACP_CMD_MAX_SIZE                                                   \
    (ACP_COMMAND_STR_SIZE + ACP_PAYLOAD1_SIZE + ACP_PAYLOAD2_SIZE + 4)
/** Maximum size for the checksum string. */
// #define ACP_CHECKSUM_STR_SIZE 2
/** Maximum size for response payload (in bytes) */
#define ACP_RESP_PAYLOAD_SIZE 256
/** Maximum size for response message (in bytes) */
#define ACP_RESP_MAX_SIZE (12 + ACP_RESP_PAYLOAD_SIZE)

/**
 * @brief Enum defining the available commands.
 *
 * First command must have value `0`.
 */
typedef enum {
    ACP_CMD_PING = 0,
    ACP_CMD_MEM,
    ACP_CMD_ADC,
    ACP_CMD_SET_LED,
    ACP_CMD_COUNT,  /**< Total number of commands. */
    ACP_CMD_INVALID /**< Invalid command. */
} acp_command_type_t;


/**
 * @brief Structure representing a serial communication command.
 */
typedef struct {
    acp_command_type_t type;          /**< The command type. */
    char payload1[ACP_PAYLOAD1_SIZE]; /**< The first payload data. */
    char payload2[ACP_PAYLOAD2_SIZE]; /**< The second payload data. */
    // uint8_t checksum; /**< Checksum for data integrity verification. */
} acp_command_t;

/**
 * @brief Enum defining response types.
 */
typedef enum {
    ACP_RESP_ACK = 0,
    ACP_RESP_TEXT,
    ACP_RESP_DATA,
    ACP_RESP_COUNT,
} acp_response_type_t;

/**
 * @brief Struct representing a serial communication response.
 *
 * The payload is sent as raw binary data.
 */
typedef struct {
    acp_response_type_t type;
    uint8_t payload_len_bytes;
    /** Raw response payload.
     * If original size is larger than 8-bits, the caller is responsible for
     * proper alignment, endianness, etc.
     */
    uint8_t payload[ACP_RESP_PAYLOAD_SIZE];
} acp_response_t;

/**
 * @brief Creates a command string for serial communication from a acp_command_t
 structure.
 *
 * @param buffer The buffer to store the formatted command string.
 * @param cmd The `acp_command_t` structure value.
 * @return true if creation was successful, false otherwire.
 */
// bool acp_create_command(char *buffer, acp_command_t cmd);

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

bool acp_build_response(acp_response_t *resp, acp_response_type_t type,
                        uint8_t payload_len_bytes, const uint8_t *payload);

/**
 * @brief Creates a response string for serial communication from a
 * `acp_response_t` structure.
 *
 * @param buffer The buffer to store the formatted response. Should have at
 * least `resp.payload_len_bytes + 12` capacity.
 * @param resp Pointer to the `acp_response_t` structure.
 * @return The number of bytes written to `buffer`.
 */
size_t acp_format_response(uint8_t *buffer, acp_response_t *resp);

#endif // ACP_SERIAL_COM_H
