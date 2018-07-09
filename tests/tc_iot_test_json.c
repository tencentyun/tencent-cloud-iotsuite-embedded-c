#include "tc_iot_inc.h"
#include "tc_iot_unit_test.h"

TEST(testjson, test_json_unescape)
{
    char output[1024];
    int ret = 0;

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
    (void)ret;
}


TEST(testjson, test_json_escape)
{
    char output[1024];
    int ret = 0;

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
    (void)ret;
}

TEST(testjson, test_json_writer)
{
    char buffer[1024];
    const char * expected = "{\"key_int\":127,\"key_nint\":-32500,\"key_float\":123.456000,\"obj\":{\"key_bool\":false,\"key_string\":\"This is a string\"}}";
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;
    CHECK_EQUAL(tc_iot_json_writer_open(w, buffer, sizeof(buffer)), 1);
    tc_iot_json_writer_int(w, "key_int", 127);
    tc_iot_json_writer_int(w, "key_nint", -32500);
    tc_iot_json_writer_decimal(w, "key_float", 123.456);
    tc_iot_json_writer_object_begin(w, "obj");
    tc_iot_json_writer_bool(w, "key_bool", false);
    tc_iot_json_writer_string(w, "key_string", "This is a string");
    tc_iot_json_writer_object_end(w);
    CHECK_EQUAL(tc_iot_json_writer_close(w), 1);
    /* tc_iot_hal_printf("\n%s\n",buffer); */
    STRCMP_EQUAL(buffer, expected);
}

