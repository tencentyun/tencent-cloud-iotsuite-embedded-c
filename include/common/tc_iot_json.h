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


typedef struct _tc_iot_json_writer {
    char * buffer;
    int buffer_len;
    int pos;
}tc_iot_json_writer;

int tc_iot_json_writer_open(tc_iot_json_writer * w, char * buffer, int buffer_len);
int tc_iot_json_writer_close(tc_iot_json_writer * w);
int tc_iot_json_writer_object_begin(tc_iot_json_writer * w, const char * name);
int tc_iot_json_writer_object_end(tc_iot_json_writer * w);
char * tc_iot_json_writer_buffer(tc_iot_json_writer * w);
int tc_iot_json_writer_string(tc_iot_json_writer * w, const char * name, const char * value);
int tc_iot_json_writer_int(tc_iot_json_writer * w, const char * name, long value);
int tc_iot_json_writer_decimal(tc_iot_json_writer * w, const char * name, double value);
int tc_iot_json_writer_bool(tc_iot_json_writer * w, const char * name, bool value);
int tc_iot_json_writer_null(tc_iot_json_writer * w, const char * name);

#endif /* end of include guard */
