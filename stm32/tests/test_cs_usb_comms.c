#include <string.h>

#include "unity.h"
#include "cs_usb_comms.h"
#include "cs_usb_comms_internals.h"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_clear_buffer()
{
    char test_str[] = "TEST_STRING";
    size_t test_str_size = sizeof(test_str);
    char *cmd_buffer = cs_get_remaining_cmd_buffer();

    // write something to the buffer
    strncpy(cmd_buffer, test_str, test_str_size);

    // clear buffer
    cs_clear_cmd_buffer();
    TEST_ASSERT_EACH_EQUAL_CHAR(0, cmd_buffer, CMD_BUFFER_MAX_LEN);
}

void test_update_buffer()
{
    char test_str[] = "TEST_STRING";
    size_t test_str_size = sizeof(test_str);
    char *cmd_buffer = cs_get_remaining_cmd_buffer();

    // write something to the buffer
    strncpy(cmd_buffer, test_str, test_str_size);

    cs_updated_cmd_buffer(test_str_size);
    // ensure buffer state is updated correctly
    size_t cmd_buffer_len = cs_get_cmd_buffer_len();
    TEST_ASSERT_EQUAL_size_t(test_str_size, cmd_buffer_len);

    size_t remaining_len = CMD_BUFFER_MAX_LEN - cmd_buffer_len;
    TEST_ASSERT_EQUAL_size_t(remaining_len, cs_get_remaining_cmd_buffer_len());
}

void test_find_cmd()
{
    cs_clear_cmd_buffer();

    // declare as individual chars to avoid null terminator
    char test_cmd[] = {'T', 'E', 'S', 'T', '_', 'C', 'M', 'D', '\n'};

    size_t test_cmd_size = sizeof(test_cmd);
    char *cmd_buffer = cs_get_remaining_cmd_buffer();

    strncpy(cmd_buffer, test_cmd, test_cmd_size);

    cs_updated_cmd_buffer(test_cmd_size);

    size_t cmd_len = cs_find_cmd();

    TEST_ASSERT_EQUAL_size_t(test_cmd_size, cmd_len);

    // the line-feed marks the command end (should be null-terminated)
    test_cmd[test_cmd_size-1] = '\0';

    TEST_ASSERT_EQUAL_STRING_LEN(test_cmd, cmd_buffer, cmd_len);
}

void test_find_partial_cmd()
{
    cs_clear_cmd_buffer();

    // declare as individual chars to avoid null terminator
    char test_cmd1[] = {'T', 'E', 'S', 'T', '_'};
    char test_cmd2[] = {'C', 'M', 'D', '\n'};

    size_t test_cmd_size1 = sizeof(test_cmd1);
    size_t test_cmd_size2 = sizeof(test_cmd2);

    char *cmd_buffer = cs_get_remaining_cmd_buffer();

    // part 1
    strncpy(cmd_buffer, test_cmd1, test_cmd_size1);
    cs_updated_cmd_buffer(test_cmd_size1);

    // should NOT find command
    size_t cmd_len = cs_find_cmd();
    TEST_ASSERT_EQUAL_size_t(0, cmd_len);

    // part 2
    cmd_buffer = cs_get_remaining_cmd_buffer();
    strncpy(cmd_buffer, test_cmd2, test_cmd_size2);
    cs_updated_cmd_buffer(test_cmd_size2);

    // should find  command
    cmd_len = cs_find_cmd();
    TEST_ASSERT_EQUAL_size_t(test_cmd_size1 + test_cmd_size2, cmd_len);
}

void test_parse_cmd()
{
    cs_clear_cmd_buffer();

    char test_cmd[] = {'T', 'O', 'G', 'G', 'L', 'E', '_', 'L', 'E', 'D', '\n'};

    size_t test_cmd_size = sizeof(test_cmd);
    char *cmd_buffer = cs_get_remaining_cmd_buffer();

    strncpy(cmd_buffer, test_cmd, test_cmd_size);
    cs_updated_cmd_buffer(test_cmd_size);
}

void test_shift_cmd_buffer()
{
    cs_clear_cmd_buffer();

    char original_content[] = "CMD1\nCMD2";
    char post_shift_content[] = "CMD2";

    size_t original_content_size = sizeof(original_content);
    size_t post_shift_content_size = sizeof(post_shift_content);


    char *cmd_buffer = cs_get_remaining_cmd_buffer();

    strncpy(cmd_buffer, original_content, original_content_size);
    cs_updated_cmd_buffer(original_content_size);

    size_t first_cmd_len = cs_find_cmd();

    TEST_ASSERT_EQUAL_size_t(5, first_cmd_len);

    cs_shift_cmd_buffer(first_cmd_len);
    TEST_ASSERT_EQUAL_STRING_LEN(post_shift_content, cmd_buffer, post_shift_content_size);
}

// not needed when using generate_test_runner.rb
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_clear_buffer);
    RUN_TEST(test_update_buffer);
    RUN_TEST(test_find_cmd);
    RUN_TEST(test_find_partial_cmd);
    RUN_TEST(test_shift_cmd_buffer);
    return UNITY_END();
}