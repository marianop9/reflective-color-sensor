#include <string.h>

#include "serial_com.h"
#include "unity.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_parse_command() {
    char test_buf[] = "&PING,123,321*";
    acp_command_type_t expected_cmd_type = ACP_CMD_PING;

    acp_command_t cmd;
    bool result = acp_parse_command(&cmd, test_buf, sizeof(test_buf));
    TEST_ASSERT_TRUE(result);

    TEST_ASSERT_EQUAL(expected_cmd_type, cmd.type);
    TEST_ASSERT_EQUAL_STRING("123", cmd.payload1);
    TEST_ASSERT_EQUAL_STRING("321", cmd.payload2);
}

void test_parse_command_one_payload() {
    char test_buf1[] = "&PING,123,*";
    acp_command_type_t expected_cmd_type = ACP_CMD_PING;

    acp_command_t cmd;
    bool result = acp_parse_command(&cmd, test_buf1, sizeof(test_buf1));
    TEST_ASSERT_TRUE(result);

    TEST_ASSERT_EQUAL(expected_cmd_type, cmd.type);
    TEST_ASSERT_EQUAL_STRING("123", cmd.payload1);

    char test_buf2[] = "&PING,,321*";

    result = acp_parse_command(&cmd, test_buf2, sizeof(test_buf2));
    TEST_ASSERT_TRUE(result);

    TEST_ASSERT_EQUAL(expected_cmd_type, cmd.type);
    TEST_ASSERT_EQUAL_STRING("321", cmd.payload2);
}

void test_parse_command_no_payload() {
    char test_buf[] = "&PING,,*";
    acp_command_type_t expected_cmd_type = ACP_CMD_PING;

    acp_command_t cmd;
    bool result = acp_parse_command(&cmd, test_buf, sizeof(test_buf));
    TEST_ASSERT_TRUE(result);

    TEST_ASSERT_EQUAL(expected_cmd_type, cmd.type);
    TEST_ASSERT_EACH_EQUAL_CHAR('\0', cmd.payload1, sizeof(cmd.payload1));
    TEST_ASSERT_EACH_EQUAL_CHAR('\0', cmd.payload2, sizeof(cmd.payload2));
}

void test_format_text_response() {
    acp_response_t resp = {
        .type = ACP_RESP_TEXT,
    };

    char payload[] = "TEXT_RESPONSE";
    // null terminator is not included:
    resp.payload_len_bytes = strlen(payload);
    memcpy(resp.payload, payload, resp.payload_len_bytes);

    char expected_resp_str[] = "&TEXT,13;TEXT_RESPONSE";

    uint8_t buffer[256];
    bool result = acp_format_response(buffer, &resp);
    TEST_ASSERT_TRUE(result);

    TEST_ASSERT_EQUAL_STRING_LEN(expected_resp_str, buffer,
                                 strlen(expected_resp_str));
}

void test_format_data_response() {
    acp_response_t resp = {
        .type = ACP_RESP_DATA,
    };

    uint8_t payload[] = {0xff, 0x12, 0x32, 0xf9, 0x33, 0x26};
    size_t payload_length = sizeof(payload);
    resp.payload_len_bytes = payload_length;
    memcpy(resp.payload, payload, resp.payload_len_bytes);

    char expected_resp_header[] = "&DATA,6;";
    size_t expected_resp_header_length = strlen(expected_resp_header);

    uint8_t buffer[256];
    bool result = acp_format_response(buffer, &resp);
    TEST_ASSERT_TRUE(result);

    // test matching header ("&type,size,")
    TEST_ASSERT_EQUAL_STRING_LEN(expected_resp_header, buffer,
                                 expected_resp_header_length);

    // test matching payload (raw bytes)
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, buffer + expected_resp_header_length,
                                  payload_length);
}

// extern bool uint8_to_str(uint8_t num, char *out_buf);

// void test() {
//     char buf[3];
//     uint8_to_str(212, buf);
//     TEST_ASSERT_EQUAL_STRING_LEN("212", buf,3);
// }

// void test_clear_buffer()
// {
//     char test_str[] = "TEST_STRING";
//     size_t test_str_size = sizeof(test_str);
//     char *cmd_buffer = cs_get_free_cmd_buffer();

//     // write something to the buffer
//     strncpy(cmd_buffer, test_str, test_str_size);

//     // clear buffer
//     cs_clear_cmd_buffer();
//     TEST_ASSERT_EACH_EQUAL_CHAR(0, cmd_buffer, CMD_BUFFER_MAX_LEN);
// }

// void test_update_buffer()
// {
//     char test_str[] = "TEST_STRING";
//     size_t test_str_size = sizeof(test_str);
//     char *cmd_buffer = cs_get_free_cmd_buffer();

//     // write something to the buffer
//     strncpy(cmd_buffer, test_str, test_str_size);

//     cs_updated_cmd_buffer(test_str_size);
//     // ensure buffer state is updated correctly
//     size_t cmd_buffer_len = cs_get_cmd_buffer_len();
//     TEST_ASSERT_EQUAL_size_t(test_str_size, cmd_buffer_len);

//     size_t remaining_len = CMD_BUFFER_MAX_LEN - cmd_buffer_len;
//     TEST_ASSERT_EQUAL_size_t(remaining_len, cs_get_free_cmd_buffer_len());
// }

// void test_find_cmd()
// {
//     cs_clear_cmd_buffer();

//     // declare as individual chars to avoid null terminator
//     char test_cmd[] = {'T', 'E', 'S', 'T', '_', 'C', 'M', 'D', '\n'};

//     size_t test_cmd_size = sizeof(test_cmd);
//     char *cmd_buffer = cs_get_free_cmd_buffer();

//     strncpy(cmd_buffer, test_cmd, test_cmd_size);

//     cs_updated_cmd_buffer(test_cmd_size);

//     size_t cmd_len = cs_find_cmd();

//     TEST_ASSERT_EQUAL_size_t(test_cmd_size, cmd_len);

//     // the line-feed marks the command end (should be null-terminated)
//     test_cmd[test_cmd_size - 1] = '\0';

//     TEST_ASSERT_EQUAL_STRING_LEN(test_cmd, cmd_buffer, cmd_len);
// }

// void test_find_partial_cmd()
// {
//     cs_clear_cmd_buffer();

//     // declare as individual chars to avoid null terminator
//     char test_cmd1[] = {'T', 'E', 'S', 'T', '_'};
//     char test_cmd2[] = {'C', 'M', 'D', '\n'};

//     size_t test_cmd_size1 = sizeof(test_cmd1);
//     size_t test_cmd_size2 = sizeof(test_cmd2);

//     char *cmd_buffer = cs_get_free_cmd_buffer();

//     // part 1
//     strncpy(cmd_buffer, test_cmd1, test_cmd_size1);
//     cs_updated_cmd_buffer(test_cmd_size1);

//     // should NOT find command
//     size_t cmd_len = cs_find_cmd();
//     TEST_ASSERT_EQUAL_size_t(0, cmd_len);

//     // part 2
//     cmd_buffer = cs_get_free_cmd_buffer();
//     strncpy(cmd_buffer, test_cmd2, test_cmd_size2);
//     cs_updated_cmd_buffer(test_cmd_size2);

//     // should find  command
//     cmd_len = cs_find_cmd();
//     TEST_ASSERT_EQUAL_size_t(test_cmd_size1 + test_cmd_size2, cmd_len);
// }

// void test_parse_cmd()
// {
//     cs_clear_cmd_buffer();

//     // null-terminated command. Don't need to call `cs_find_cmd()`
//     char test_cmd[] = "TOGGLE_LED";

//     size_t test_cmd_size = sizeof(test_cmd);

//     char *cmd_buffer = cs_get_free_cmd_buffer();
//     strncpy(cmd_buffer, test_cmd, test_cmd_size);
//     cs_updated_cmd_buffer(test_cmd_size);

//     cs_command cmd = cs_parse_cmd();
//     TEST_ASSERT_NOT_EQUAL(CS_COMMAND_ERR, cmd);
//     TEST_ASSERT_EQUAL(CS_COMMAND_TOGGLE_LED, cmd);
// }

// void test_parse_args()
// {
//     cs_clear_cmd_buffer();
//     char args[] = "123 321";

//     int arg_count = cs_parse_args(args);
//     TEST_ASSERT_EQUAL(2, arg_count);

//     TEST_ASSERT_EQUAL_UINT32(123, cs_get_arg(0));
//     TEST_ASSERT_EQUAL_UINT32(321, cs_get_arg(1));

//     char args_spaced[] = " 321 123";

//     arg_count = cs_parse_args(args_spaced);
//     TEST_ASSERT_EQUAL(2, arg_count);

//     TEST_ASSERT_EQUAL_UINT32(321, cs_get_arg(0));
//     TEST_ASSERT_EQUAL_UINT32(123, cs_get_arg(1));
// }

// void test_check_for_command()
// {
//     cs_clear_cmd_buffer();

//     char test_cmd[] = {'S', 'E', 'T', '_', 'L', 'E', 'D', ' ', '1', '2', '3',
//     ' ', '3', '2', '1', ' ', '9', '\n'}; size_t test_cmd_size =
//     sizeof(test_cmd);

//     char *cmd_buffer = cs_get_free_cmd_buffer();
//     strncpy(cmd_buffer, test_cmd, test_cmd_size);
//     cs_updated_cmd_buffer(test_cmd_size);

//     cs_command cmd;
//     bool found_cmd = cs_check_for_command(&cmd);

//     TEST_ASSERT_TRUE(found_cmd);
//     TEST_ASSERT_EQUAL(CS_COMMAND_SET_LED, cmd);
//     TEST_ASSERT_EQUAL_UINT32(123, cs_get_arg(0));
//     TEST_ASSERT_EQUAL_UINT32(321, cs_get_arg(1));
// }

// void test_shift_cmd_buffer()
// {
//     cs_clear_cmd_buffer();

//     char original_content[] = "CMD1\nCMD2";
//     char post_shift_content[] = "CMD2";

//     size_t original_content_size = sizeof(original_content);
//     size_t post_shift_content_size = sizeof(post_shift_content);

//     char *cmd_buffer = cs_get_free_cmd_buffer();

//     strncpy(cmd_buffer, original_content, original_content_size);
//     cs_updated_cmd_buffer(original_content_size);

//     size_t first_cmd_len = cs_find_cmd();

//     TEST_ASSERT_EQUAL_size_t(5, first_cmd_len);

//     cs_shift_cmd_buffer(first_cmd_len);
//     TEST_ASSERT_EQUAL_STRING_LEN(post_shift_content, cmd_buffer,
//     post_shift_content_size);
// }

// void test_build_text_response()
// {
//     cs_response_msg resp = {0};
//     char *str = "TEST_STRING";

//     cs_build_text_response(&resp, str);

//     TEST_ASSERT_EQUAL_size_t(strlen(str), resp.len);

//     TEST_ASSERT_EQUAL_STRING_LEN(str, resp.payload, resp.len);
// }

// void test_build_data_response()
// {
//     cs_response_msg resp = {0};
//     uint16_t data[] = {1, 2, 3, 4, 5, 6, 7, 8};
//     size_t len = sizeof(data) / sizeof(data[0]);

//     cs_build_data_response(&resp, data, len);

//     TEST_ASSERT_EQUAL_size_t(len, resp.len);

//     // original data and payload aren't the same datatype, can't use
//     TEST_ASSERT_EQUAL_UINT16_ARRAY TEST_ASSERT_EQUAL_MEMORY_ARRAY(data,
//     resp.payload, 1, resp.len);
// }

// not needed when using generate_test_runner.rb
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_command);
    RUN_TEST(test_parse_command_one_payload);
    RUN_TEST(test_parse_command_no_payload);
    RUN_TEST(test_format_text_response);
    RUN_TEST(test_format_data_response);
    return UNITY_END();
}