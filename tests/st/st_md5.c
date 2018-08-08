#include "tc_iot_inc.h"

void test(const char * data, const char * expected) {
    int len = 0;
    unsigned char md5sum[TC_IOT_MD5_DIGEST_SIZE];
    char md5sum_hexstr[2*TC_IOT_MD5_DIGEST_SIZE + 1];
    if (data) {
        len = strlen(data);
    }
    tc_iot_md5_digest((const unsigned char *)data, len, md5sum);
    if (0 == strcmp(expected, tc_iot_util_byte_to_hex(md5sum, sizeof(md5sum), md5sum_hexstr, sizeof(md5sum_hexstr)))) {
        tc_iot_hal_printf("%s == %s\n", expected,  md5sum_hexstr);
    } else {
        tc_iot_hal_printf("%s != %s\n", expected,  md5sum_hexstr);
    }
}

int main(int argc, char const* argv[])
{
    test("The quick brown fox jumps over the lazy dog",
            "9e107d9d372bb6826bd81d3542a419d6");
    test("The quick brown fox jumps over the lazy dog.",
            "e4d909c290d0fb1ca068ffaddf22cbd0");
    test("",
            "d41d8cd98f00b204e9800998ecf8427e");
    test(NULL,
            "d41d8cd98f00b204e9800998ecf8427e");
    return 0;
}

