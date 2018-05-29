#include "tc_iot_inc.h"
#include "tc_iot_unit_test.h"

tc_unit_test_state_t g_tc_unit_test_state = {0,0,0,0};

TEST(testbase64, test_base64_encode);
TEST(testbase64, test_base64_decode);
TEST(test_ya_buffer, test_ya_buffer_all);
TEST(testjson, test_json_unescape);
TEST(testjson, test_json_escape);
TEST(testjson, test_json_writer);
TEST(testbit, test_bit_all);


int main(int ac, char **argv) {
    tc_iot_set_log_level(TC_IOT_LOG_LEVEL_FATAL);

    TC_IOT_UT_PRINT_STAT_HEADER();
    TC_IOT_UT_RUN_TEST(tc_base64, test_base64_encode, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(tc_base64, test_base64_decode, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(test_ya_buffer, test_ya_buffer_all, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(testjson, test_json_unescape, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(testjson, test_json_escape, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(testjson, test_json_writer, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(testbit, test_bit_all, g_tc_unit_test_state);

    TC_IOT_UT_PRINT_STAT_FOOTER(g_tc_unit_test_state);
}

