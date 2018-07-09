#include "tc_iot_inc.h"

void test(const char * data, const char * secret) {
    unsigned char sha256_digest_old[TC_IOT_SHA256_DIGEST_SIZE];
    char sha256_digest_str_old[TC_IOT_SHA256_DIGEST_SIZE*2+1];

    unsigned char sha256_digest_new[TC_IOT_SHA256_DIGEST_SIZE];
    char sha256_digest_str_new[TC_IOT_SHA256_DIGEST_SIZE*2+1];

    tc_iot_hal_printf("\n");
    tc_iot_hmac_sha256((unsigned char *)data, strlen(data), (const unsigned char *)secret, strlen(secret), (unsigned char *)sha256_digest_old);
    tc_iot_hal_printf("\n");
    tc_iot_calc_sign(sha256_digest_new, sizeof(sha256_digest_str_new),secret, 
            "a=%s&b=%d&c=%d%%", "abcae@test", 1786439041, 1523260742);
    tc_iot_hal_printf("\n");

    if (0 == strcmp(
                tc_iot_util_byte_to_hex(sha256_digest_new, sizeof(sha256_digest_new), sha256_digest_str_new, sizeof(sha256_digest_str_new))
                , tc_iot_util_byte_to_hex(sha256_digest_old, sizeof(sha256_digest_old), sha256_digest_str_old, sizeof(sha256_digest_str_old)))) {
        tc_iot_hal_printf("%s == %s\n", sha256_digest_str_new,  sha256_digest_str_old);
    } else {
        tc_iot_hal_printf("%s != %s\n", sha256_digest_str_new,  sha256_digest_str_old);
    }


}

int main(int argc, char const* argv[])
{
    unsigned char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];

    char buffer[1024];
    int buffer_len = sizeof(buffer);
    char buffer_new[1024];
    int buffer_new_len = sizeof(buffer_new);

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

    // printf("%s\n", buffer);

    tc_iot_hal_snprintf(buffer_new, buffer_new_len, 
            format,
            var_string,
            var_char,
            var_int,
            var_uint,
            var_oct,
            var_ulong
            );

    printf("%s\n", buffer_new);
    tc_iot_hmac_sha256((unsigned char *)buffer_new, strlen(buffer_new), (const unsigned char *)secret, strlen(secret), (unsigned char *)sha256_digest);
    ret = tc_iot_base64_encode(sha256_digest, sizeof(sha256_digest), buffer_new, buffer_new_len);
    buffer_new[ret] = '\0';


    tc_iot_hal_printf("%s\n%s\n", buffer, buffer_new);
    return 0;
}

