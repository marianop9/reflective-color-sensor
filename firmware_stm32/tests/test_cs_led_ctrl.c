#include "unity.h"

#include "cs_led_ctrl.h"

static uint16_t pwm_max = 10;
static uint16_t high = 6;
static uint16_t low = 2;

void setUp(void)
{
    led_ctrl_init(pwm_max);
}

void tearDown(void)
{
    // clean stuff up here
}

void test_led_ctrl_init()
{
    const uint16_t *buffer = led_ctrl_get_buffer();
    uint16_t len = led_ctrl_get_buffer_len();

    int led_bits = 24 * LED_CTRL_NUM_LEDS;
    // led bits set low
    TEST_ASSERT_EACH_EQUAL_UINT16(low, buffer, led_bits);
    // padding set to 0
    TEST_ASSERT_EACH_EQUAL_UINT16(0, buffer + led_bits, len - led_bits);
}

void test_led_ctrl_set_buffer_one_led()
{
    uint8_t r = 0xff, g = 0x00, b = 0x00;

    const uint16_t *buffer = led_ctrl_get_buffer();

    // configure second LED
    int result = led_ctrl_set_buffer(2, r, g, b);
    TEST_ASSERT_EQUAL(0, result);

    // first 24 bits untouched
    TEST_ASSERT_EACH_EQUAL_UINT16(low, buffer, 24);
    // green bits low
    TEST_ASSERT_EACH_EQUAL_UINT16(low, buffer + 24, 8);
    // red bits high
    TEST_ASSERT_EACH_EQUAL_UINT16(high, buffer + 24 + 8, 8);
    // blue bits low
    TEST_ASSERT_EACH_EQUAL_UINT16(low, buffer + 24 + 16, 8);

    // padding left untouched
    int led_bits = 24 * LED_CTRL_NUM_LEDS;
    TEST_ASSERT_EACH_EQUAL_UINT16(
        0,
        buffer + led_bits,
        led_ctrl_get_buffer_len() - led_bits);
}

void test_led_ctrl_set_buffer_all_leds()
{
    uint8_t r = 0xff, g = 0x00, b = 0x00;

    const uint16_t *buffer = led_ctrl_get_buffer();

    // configure second LED
    int result = led_ctrl_set_buffer(0, r, g, b);
    TEST_ASSERT_EQUAL(0, result);

    // LED 1: green bits low
    TEST_ASSERT_EACH_EQUAL_UINT16(low, buffer, 8);
    // LED 1: red bits high
    TEST_ASSERT_EACH_EQUAL_UINT16(high, buffer + 8, 8);
    // LED 1: blue bits low
    TEST_ASSERT_EACH_EQUAL_UINT16(low, buffer + 16, 8);
    // LED 2: green bits low
    TEST_ASSERT_EACH_EQUAL_UINT16(low, buffer + 24, 8);
    // LED 2: red bits high
    TEST_ASSERT_EACH_EQUAL_UINT16(high, buffer + 24 + 8, 8);
    // LED 2: blue bits low
    TEST_ASSERT_EACH_EQUAL_UINT16(low, buffer + 24 + 16, 8);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_led_ctrl_init);
    RUN_TEST(test_led_ctrl_set_buffer_one_led);
    RUN_TEST(test_led_ctrl_set_buffer_all_leds);
    return UNITY_END();
}