extern "C" {
#include "tc_iot_inc.h"
}
#include "gtest/gtest.h"

TEST(tc_iot_http_utils_testcases, tc_iot_calc_sign)
{
    unsigned char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];

    char buffer[1024];
    int buffer_len = sizeof(buffer);
    char buffer_new[1024];
    int buffer_new_len = sizeof(buffer_new);

    memset(buffer, 0, buffer_len);
    memset(buffer_new, 0, buffer_new_len);

    const char * secret = "test";
    int ret = 0;
    const char * var_string = "string%20_abcd@441255!##!$DSS";
    char var_char = 'Z';
    int var_int = 0x12345678;
    unsigned int var_uint = 0xFFFFFFFF;
    int var_oct = 01234567;
    unsigned int var_ulong = 0x87654321;

    const char * format = "StringParam=%s&Char=%c&Int=%d&Long=%ld&Octo=%o&timestamp=0x%x";

    ret = tc_iot_calc_sign(sha256_digest,
            sizeof(sha256_digest),
            secret, 
            format,
            var_string,
            var_char,
            var_int,
            var_uint,
            var_oct,
            var_ulong
            );
    ret = tc_iot_base64_encode(sha256_digest, sizeof(sha256_digest), buffer, buffer_len);
    buffer[ret] = '\0';

    tc_iot_hal_snprintf(buffer_new, buffer_new_len, 
            format,
            var_string,
            var_char,
            var_int,
            var_uint,
            var_oct,
            var_ulong
            );
    tc_iot_hmac_sha256((unsigned char *)buffer_new, strlen(buffer_new), (const unsigned char *)secret, strlen(secret), (unsigned char *)sha256_digest);
    ret = tc_iot_base64_encode(sha256_digest, sizeof(sha256_digest), buffer_new, buffer_new_len);
    buffer_new[ret] = '\0';

    //ZbUfimvkTsysAqe/Vdchuc+shMW+fVF48sX5rS8zb10=

    ASSERT_STREQ(buffer, buffer_new);
}


