#ifndef TC_IOT_JSON_H
#define TC_IOT_JSON_H

#include "tc_iot_inc.h"

int tc_iot_jsoneq(const char *json, jsmntok_t *tok, const char *s);
int tc_iot_jsoneq_len(const char *json, const jsmntok_t *tok, const char *s,
                      int len);
int tc_iot_json_unescape(char *dest, int dest_len, const char *src, int src_len);
char * tc_iot_json_inline_escape(char *dest, int dest_len, const char *src);

int tc_iot_json_escape(char *dest, int dest_len, const char *src, int src_len);
int tc_iot_json_parse(const char * json, int json_len, jsmntok_t * tokens, int token_count);
const char * tc_iot_json_token_type_str(int type);
void tc_iot_json_print_node(const char *prefix, const char *json, const jsmntok_t *node_root, int node_index);

int tc_iot_json_find_token(const char *json, const jsmntok_t *root_token,
                           int count, const char *path, char *result,
                           int result_len);

#define TC_IOT_JSON_NULL "null"
#define TC_IOT_JSON_TRUE "true"
#define TC_IOT_JSON_FALSE "false"

typedef enum _tc_iot_type_e{
    TC_IOT_UNKNOW_TYPE,

    TC_IOT_INT8,
    TC_IOT_INT16,
    TC_IOT_INT32,
    TC_IOT_UINT8,
    TC_IOT_UINT16,
    TC_IOT_UINT32,
    TC_IOT_FLOAT,
    TC_IOT_DOUBLE,
    TC_IOT_BOOL,

    TC_IOT_STRING,
    TC_IOT_ARRAY,
    TC_IOT_OBJECT,
}tc_iot_type_e;

typedef void * pointer;

typedef union _tc_iot_variant_data{
    int8_t   as_int8_t;
    int16_t  as_int16_t;
    int32_t  as_int32_t;
    uint8_t  as_uint8_t;
    uint16_t as_uint16_t;
    uint32_t as_uint32_t;
    bool     as_bool;
    float    as_float;
    double   as_double;
    char *   as_string;
    void *   as_array;
    void *   as_object;
}tc_iot_variant_data;

typedef struct _tc_iot_property{
    const char * key;
    tc_iot_variant_data data;
    int length;
    tc_iot_type_e type;
}tc_iot_property;

#define DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(sys_type, iot_type) \
tc_iot_property tc_iot_property_ ## sys_type(const char * key, sys_type val)

#define DEFINE_TC_IOT_PROPERTY_FUNC(sys_type, iot_type) \
tc_iot_property tc_iot_property_ ## sys_type(const char * key, sys_type val) { \
    tc_iot_property prop; \
    prop.key = key; \
    prop.length = sizeof(sys_type); \
    prop.data.as_ ## sys_type = val;\
    prop.type = iot_type; \
    return prop; \
} \

#define TC_IOT_PROPERTY(key,val,sys_type) tc_iot_property_ ## sys_type(key, val)

#define TC_IOT_PROPERTY_REF(key,val,type,length) tc_iot_property_ref(key, val, type, length)

DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(int8_t,  TC_IOT_INT8);
DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(int16_t, TC_IOT_INT16);
DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(int32_t, TC_IOT_INT32);
DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(uint8_t,  TC_IOT_UINT8);
DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(uint16_t, TC_IOT_UINT16);
DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(uint32_t, TC_IOT_UINT32);
DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(bool, TC_IOT_BOOL);
DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(float, TC_IOT_FLOAT);
DEFINE_TC_IOT_PROPERTY_FUNC_PROTO(double, TC_IOT_DOUBLE);

tc_iot_property tc_iot_property_ref(const char * key, void * ptr, tc_iot_type_e type, int length);
int tc_iot_json_property_printf(char * buffer, int len, int count, ... );

#define TC_IOT_MAX_JSON_TOKEN_COUNT     120

#endif /* end of include guard */
