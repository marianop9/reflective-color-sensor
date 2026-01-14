/** TODO!
 * 
 * Hacer un test como la gente con cmake+unity
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "serial_com.h"

extern uint8_t calculate_checksum(const char *);

void cmd_without_checksum(char *str);

void test_create_and_parse_command() {
    acp_command_t cmd = {
        .type = ACP_CMD_PING, .payload1 = "payload1", .payload2 = "payload2"};

    char buffer[ACP_CMD_BUFFER_SIZE] = {0};

    bool success = acp_create_command(buffer, cmd);
    assert(success && "Failed to create command.");

    acp_command_t parsed_cmd;
    success = acp_parse_command(&parsed_cmd, buffer, strlen(buffer));
    assert(success && "Failed to parse command.");

    cmd_without_checksum(buffer);
    const uint8_t checksum = calculate_checksum(buffer);

    assert(parsed_cmd.type == cmd.type);
    assert(strcmp(parsed_cmd.payload1, cmd.payload1) == 0);
    assert(strcmp(parsed_cmd.payload2, cmd.payload2) == 0);
    assert(parsed_cmd.checksum == checksum);
}

void test_create_response() {
    char test_payload[] = "response123";
    int test_payload_len = strlen(test_payload);
    
    acp_response_t resp = {.type = ACP_RESP_TEXT};
    memcpy(resp.payload, test_payload, test_payload_len);
    resp.len_bytes = test_payload_len;

    uint8_t buffer[128] = {0};

    bool success = acp_create_response(buffer, &resp);
    assert(success && "Failed to create response.");

    assert(buffer[0] == ACP_START_CHAR);
    assert(strncmp(buffer+1, "TEXT", 4) == 0);
    assert(buffer[1+4] == ',');
    assert(buffer[5+1] == resp.len_bytes);
    assert(buffer[6+1] == ',');
    assert(strncmp(buffer+7+1,  test_payload, test_payload_len) == 0);
    assert(buffer[8+test_payload_len] == ACP_END_CHAR);
}

int main() {
    test_create_and_parse_command();

    test_create_response();

    printf("All tests passed.\n");
    return 0;
}

void cmd_without_checksum(char *str) {
    size_t len = strlen(str);
    if (len > 2) {
        str[len - 2] = '\0';
    } else if (len > 0) {
        str[0] = '\0';
    }
}