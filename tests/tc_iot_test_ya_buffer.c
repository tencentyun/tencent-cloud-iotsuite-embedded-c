#include "tc_iot_inc.h"
#include "tc_iot_unit_test.h"

TEST(test_ya_buffer, test_ya_buffer_all)  
{
    tc_iot_yabuffer_t ya_buffer;
    char buffer[255];
    int buffer_len = sizeof(buffer);
    int ret;
    int space_left;
    ret =  tc_iot_yabuffer_init(NULL, NULL, 0);
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);
    ret =  tc_iot_yabuffer_init(&ya_buffer, NULL, 0);
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);

    ret = tc_iot_yabuffer_left(NULL);
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);
    char * current = tc_iot_yabuffer_current(NULL);
    POINTERS_EQUAL(current, NULL);

    ret = tc_iot_yabuffer_forward(NULL,1);
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);

    ret = tc_iot_yabuffer_append(NULL,"");
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);

    ret =  tc_iot_yabuffer_init(&ya_buffer, buffer, 0);
    CHECK_EQUAL(TC_IOT_INVALID_PARAMETER, ret);

    ret =  tc_iot_yabuffer_init(&ya_buffer, buffer, buffer_len);
    CHECK_EQUAL(TC_IOT_SUCCESS, ret);
    space_left = tc_iot_yabuffer_left(&ya_buffer);
    CHECK_EQUAL(space_left, buffer_len);

    const char * test_str = "Some test";
    ret = tc_iot_yabuffer_append(&ya_buffer, test_str);
    CHECK_EQUAL(ret, strlen(test_str));

    ret = tc_iot_yabuffer_reset(&ya_buffer);
    CHECK_EQUAL(ret, TC_IOT_SUCCESS);

    space_left = tc_iot_yabuffer_left(&ya_buffer);
    CHECK_EQUAL(space_left, buffer_len);
} 

