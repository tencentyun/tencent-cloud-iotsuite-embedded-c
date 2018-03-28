#include "tc_iot_inc.h"
#include "tc_iot_unit_test.h"


TEST(testbit, test_bit_all)
{
    unsigned char bitmap[16];
    int i = 0;
    memset(bitmap, 0, sizeof(bitmap));
    for (i = 0; i < 8*sizeof(bitmap); i++) {
        CHECK_EQUAL(0, TC_IOT_BIT_GET(&bitmap[0], i));
    }
    memset(bitmap, 0xFF, sizeof(bitmap));
    for (i = 0; i < 8*sizeof(bitmap); i++) {
        CHECK_EQUAL(1, TC_IOT_BIT_GET(&bitmap[0], i));
    }

    memset(bitmap, 0xAA, sizeof(bitmap));
    for (i = 0; i < 8*sizeof(bitmap); i++) {
        if (i&0x1) {
            CHECK_EQUAL(1, TC_IOT_BIT_GET(&bitmap[0], i));
        } else {
            CHECK_EQUAL(0, TC_IOT_BIT_GET(&bitmap[0], i));
        }
    }
}

