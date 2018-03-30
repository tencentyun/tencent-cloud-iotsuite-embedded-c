#include "tc_iot_inc.h"
#include "tc_iot_unit_test.h"

TEST(testjson, test_json_unescape)
{
    char output[1024];
    int buffer_len = sizeof(output);
    int ret;

    const char * input ;

    input = "good";
    ret = tc_iot_json_unescape(output, sizeof(output), input, strlen(input));
    /* printf("'%s' -> '%s'\n", input, output);*/
    STRCMP_EQUAL(output, "good");

    input = "\\\\,\\/good";
    ret = tc_iot_json_unescape(output, sizeof(output), input, strlen(input));
    /* printf("'%s' -> '%s'\n", input, output);*/
    STRCMP_EQUAL(output, "\\,/good");

    input = "\\r\\n\\b\\t\\fgood";
    ret = tc_iot_json_unescape(output, sizeof(output), input, strlen(input));
    /* printf("'%s' -> '%s'\n", input, output);*/
    STRCMP_EQUAL(output, "\r\n\b\t\fgood");

    input = "\\u000agood\\u000d\\u000a";
    ret = tc_iot_json_unescape(output, sizeof(output), input, strlen(input));
    /* printf("'%s' -> '%s'\n", input, output); */
    STRCMP_EQUAL(output, "\ngood\r\n");
}


TEST(testjson, test_json_escape)
{
    char output[1024];
    int buffer_len = sizeof(output);
    int ret;

    const char * input ;

    input = "good";
    ret = tc_iot_json_escape(output, sizeof(output), input, strlen(input));
    /* printf("'%s' -> '%s'\n", input, output);*/
    STRCMP_EQUAL(output, "good");

    input = "\\,/\"good";
    ret = tc_iot_json_escape(output, sizeof(output), input, strlen(input));
    /* printf("'%s' -> '%s'\n", input, output); */
    STRCMP_EQUAL(output,"\\\\,\\/\\\"good");

    input = "\r\n\b\t\fgood";
    ret = tc_iot_json_escape(output, sizeof(output), input, strlen(input));
    /*printf("'%s' -> '%s'\n", input, output);*/
    STRCMP_EQUAL(output, "\\r\\n\\b\\t\\fgood");

    input = "\ngood\r\n";
    ret = tc_iot_json_escape(output, sizeof(output), input, strlen(input));
    /* printf("'%s' -> '%s'\n", input, output); */
    STRCMP_EQUAL(output, "\\ngood\\r\\n");
}

TEST(testjson, test_json_property)
{
    char buffer[1024];
    const char * expected = "{\"key_int8_t\":127,\"key_int16_t\":32500,\"key_int32_t\":80000,\"key_uint8_t\":128,\"key_uint16_t\":65535,\"key_uint32_t\":80000,\"key_float\":10.111000,\"key_double\":11112110.111000,\"key_bool\":true,\"key_string\":\"This is a string\"}";
    char * test = "This is a string";
    tc_iot_property prop_int8 = TC_IOT_PROPERTY("key_int8_t", 127, int8_t);
    tc_iot_property prop_int16 = TC_IOT_PROPERTY("key_int16_t", 32500, int16_t);
    tc_iot_property prop_int32 = TC_IOT_PROPERTY("key_int32_t", 80000, int32_t);
    tc_iot_property prop_uint8 = TC_IOT_PROPERTY("key_uint8_t", 128, uint8_t);
    tc_iot_property prop_uint16 = TC_IOT_PROPERTY("key_uint16_t", 65535, uint16_t);
    tc_iot_property prop_uint32 = TC_IOT_PROPERTY("key_uint32_t", 80000, uint32_t);
    tc_iot_property prop_float = TC_IOT_PROPERTY("key_float", 10.111, float);
    tc_iot_property prop_double = TC_IOT_PROPERTY("key_double", 11112110.111, double);
    tc_iot_property prop_bool = TC_IOT_PROPERTY("key_bool", true, bool);
    tc_iot_property prop_str = TC_IOT_PROPERTY_REF("key_string", test, TC_IOT_STRING, strlen(test));

    /*int ret = tc_iot_json_property_printf(buffer, sizeof(buffer), 10,*/
        /*&prop_int8, &prop_int16, &prop_int32, &prop_uint8, &prop_uint16, &prop_uint32,*/
        /*&prop_float, &prop_double, &prop_bool, &prop_str);*/
    /*[>printf("json: %s\n", buffer);<]*/
    /*STRCMP_EQUAL(buffer, expected);*/
}

