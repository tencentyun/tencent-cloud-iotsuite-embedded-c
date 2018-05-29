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
    /* unsigned char test[4] = {0,0,0,0}; */
    /* #<{(| unsigned int test[] = {0,0}; |)}># */
    /* printf("seq\tbit\tset\tflip\n"); */
    /* for (i = 0; i < 8*sizeof(test); i ++) { */
    /*     printf("%d\t", i); */
    /*     TC_IOT_BIT_SET(&test[0], i); */
    /*     printf("0x%X\t", *(int *)&test[0]); */
    /*     TC_IOT_BIT_CLEAR(&test[0], i); */
    /*     printf("0x%X\t", *(int *)&test[0]); */
    /*     TC_IOT_BIT_FLIP(&test[0], i); */
    /*     printf("0x%X\n", *(int *)&test[0]); */
    /* } */
}

