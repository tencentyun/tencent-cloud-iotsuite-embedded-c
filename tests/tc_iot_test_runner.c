// #ifdef __cplusplus
// extern "C" {
// #endif

#include "tc_iot_inc.h"
#include "tc_iot_unit_test.h"

int g_tc_iot_unit_test_success = 0;
int g_tc_iot_unit_test_failed = 0;

int main(int ac, char **argv) {
    TC_IOT_UT_RUN_TEST(tc_base64, test_base64_encode);
    TC_IOT_UT_RUN_TEST(tc_base64, test_base64_decode);
    TC_IOT_UT_RUN_TEST(test_ya_buffer, test_ya_buffer_all);
    TC_IOT_UT_RUN_TEST(testjson, test_json_unescape);

    int total = g_tc_iot_unit_test_success+g_tc_iot_unit_test_failed;
    int success = g_tc_iot_unit_test_success;
    int failed = g_tc_iot_unit_test_failed;

    printf("total: %d, success: %d, failed: %d\n", total, success, failed);
}

// #ifdef __cplusplus
// }
// #endif
