#ifndef TC_IOT_UNIT_TEST_H
#define TC_IOT_UNIT_TEST_H


typedef struct _tc_unit_test_state_t {
    int success;
    int failed;
    int temp_success;
    int temp_failed;
}tc_unit_test_state_t;

extern tc_unit_test_state_t g_tc_unit_test_state;

#define TC_IOT_UT_ASSERT(cond) do { \
    if (!(cond)) { \
        g_tc_unit_test_state.failed++; \
        tc_iot_hal_printf("UT_FAILED %s:%d condition checkfailed %s\n", __FUNCTION__, __LINE__, #cond); \
    } else { \
        g_tc_unit_test_state.success++; \
    } \
} while (0)

#define CHECK_EQUAL(expected, actual) TC_IOT_UT_ASSERT(expected == actual)
#define POINTERS_EQUAL(expected, actual) TC_IOT_UT_ASSERT(expected == actual)
#define STRCMP_EQUAL(expected, actual) TC_IOT_UT_ASSERT(strcmp(expected, actual) == 0)

#define TC_IOT_UT_PRINT_STAT_HEADER() tc_iot_hal_printf("total\tsucc\tfail\tcase\n")

#define TC_IOT_UT_RUN_TEST(tg, tc, state) do {\
    state.temp_success= state.success; \
    state.temp_failed = state.failed;\
    tc(); \
    tc_iot_hal_printf("%d\t%d\t%d\t%s/%s\n", \
    state.success-state.temp_success + state.failed - state.temp_failed, \
    state.success-state.temp_success, state.failed - state.temp_failed, \
#tg, #tc); \
} while (0) \

#define TC_IOT_UT_PRINT_STAT_FOOTER(state) \
tc_iot_hal_printf("%d\t%d\t%d\tTOTAL\n", state.success+state.failed, state.success, state.failed)

/* STUB */
#define TEST_GROUP(tg)  void tg( void )
#define TEST(tg, tc) void tc ( void )

#endif /* end of include guard */
