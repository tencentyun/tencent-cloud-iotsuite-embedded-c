#ifndef TC_IOT_UNIT_TEST_H
#define TC_IOT_UNIT_TEST_H

extern int g_tc_iot_unit_test_success;
extern int g_tc_iot_unit_test_failed;

#define TC_IOT_UT_ASSERT(cond) do { \
    if (!(cond)) { \
        g_tc_iot_unit_test_failed++; \
        tc_iot_hal_printf("UT_FAILED %s:%d condition checkfailed %s\n", __FUNCTION__, __LINE__, #cond); \
    } else { \
        g_tc_iot_unit_test_success++; \
    } \
} while (0)

#define CHECK_EQUAL(expected, actual) TC_IOT_UT_ASSERT(expected == actual)
#define POINTERS_EQUAL(expected, actual) TC_IOT_UT_ASSERT(expected == actual)
#define STRCMP_EQUAL(expected, actual) TC_IOT_UT_ASSERT(strcmp(expected, actual) == 0)

#define TC_IOT_UT_RUN_TEST(tg, tc) do { tc(); } while (0)

/* STUB */
#define TEST_GROUP(tg)  int tg( void )
#define TEST(tg, tc) int tc ( void )

#endif /* end of include guard */
