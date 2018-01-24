#include "tc_iot_inc.h"
#include "tc_iot_unit_test.h"

tc_unit_test_state_t g_tc_unit_test_state = {0,0,0,0};

int main(int ac, char **argv) {
    tc_iot_set_log_level(TC_IOT_LOG_CRIT);

    TC_IOT_UT_PRINT_STAT_HEADER();
    TC_IOT_UT_RUN_TEST(tc_base64, test_base64_encode, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(tc_base64, test_base64_decode, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(test_ya_buffer, test_ya_buffer_all, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(testjson, test_json_unescape, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(testjson, test_json_escape, g_tc_unit_test_state);
    TC_IOT_UT_RUN_TEST(testjson, test_json_property, g_tc_unit_test_state);

    TC_IOT_UT_PRINT_STAT_FOOTER(g_tc_unit_test_state);
}

