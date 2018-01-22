#include "../src/common/tc_iot_json.c"
#include "CppUTest/TestHarness.h"


TEST_GROUP(testjson)  
{  

};  

TEST(testjson, tc_json_unescape)  
{  
    char output[1024];
    int buffer_len = sizeof(output);
    int ret;

    const char * input ;

    input = "good";
    ret = tc_iot_json_unescape(output, sizeof(output), input, strlen(input));
    // printf("'%s' -> '%s'\n", input, output);
    STRCMP_EQUAL(output, "good");

    input = "\\\\,\\/good";
    ret = tc_iot_json_unescape(output, sizeof(output), input, strlen(input));
    // printf("'%s' -> '%s'\n", input, output);
    STRCMP_EQUAL(output, "\\,/good");

    input = "\\r\\n\\b\\t\\fgood";
    ret = tc_iot_json_unescape(output, sizeof(output), input, strlen(input));
    // printf("'%s' -> '%s'\n", input, output);
    STRCMP_EQUAL(output, "\r\n\b\t\fgood");

    input = "\u000agood\u000d\u000a";
    ret = tc_iot_json_unescape(output, sizeof(output), input, strlen(input));
    // printf("'%s' -> '%s'\n", input, output);
    STRCMP_EQUAL(output, "\ngood\r\n");
} 

