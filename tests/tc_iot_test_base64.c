#include "tc_iot_inc.h"
#include "tc_iot_unit_test.h"


TEST(testbase64, test_base64_encode)
{
    const char * base64_in = "I am tom.";
    const char * base64_out = "SSBhbSB0b20u";
    char output[1024];
    int buffer_len = sizeof(output);
    int ret;
    ret = tc_iot_base64_encode((const unsigned char *)base64_in, strlen(base64_in), NULL, buffer_len);
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);
    ret = tc_iot_base64_encode((const unsigned char *)NULL, strlen(base64_in), NULL, buffer_len);
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);
    ret = tc_iot_base64_encode((const unsigned char *)base64_in, strlen(base64_in), output, 0);
    CHECK_EQUAL(TC_IOT_INVALID_PARAMETER, ret);

    memset(output, 0, buffer_len);
    ret = tc_iot_base64_encode((const unsigned char *)base64_in, strlen(base64_in), output, buffer_len);
    CHECK_EQUAL(strlen(output), ret);
    STRCMP_EQUAL(output, base64_out);
}

TEST(testbase64, test_base64_decode)
{
    const unsigned char * base64_in = (const unsigned char *)"I am tom.";
    const char * base64_out = "SSBhbSB0b20u";
    unsigned char output[1024];
    int buffer_len = sizeof(output);
    int ret;
    ret = tc_iot_base64_decode((const char *)base64_out, strlen(base64_out), NULL, buffer_len);
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);
    ret = tc_iot_base64_decode((const char *)NULL, strlen(base64_out), NULL, buffer_len);
    CHECK_EQUAL(TC_IOT_NULL_POINTER, ret);
    ret = tc_iot_base64_decode((const char *)base64_out, strlen(base64_out), output, 0);
    CHECK_EQUAL(TC_IOT_INVALID_PARAMETER, ret);

    memset(output, 0, buffer_len);
    ret = tc_iot_base64_decode((const char *)base64_out, strlen(base64_out), output, buffer_len);
    CHECK_EQUAL(strlen((const char *)output), ret);
    STRCMP_EQUAL((const char *)output, (const char *)base64_in);
}

