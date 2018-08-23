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


/*
 * tc_iot_json_writer 用来往指定 buffer 中写入 json 数据，
 * 写入数据时，自动对数据进行转义及 buffer 溢出判断处理。
 * 简化 json 组包逻辑。
 * */
typedef struct _tc_iot_json_writer {
    char * buffer;
    int buffer_len;
    int pos;
}tc_iot_json_writer;


/**
 * @brief 初始化 json writer 数据，指定待写入的 buffer
 *
 * @param w writer 对象
 * @param buffer 待写入的 buffer 区域
 * @param buffer_len buffer 长度
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_open(tc_iot_json_writer * w, char * buffer, int buffer_len);

/**
 * @brief 结束 json writer 写入。
 *
 * @param w writer 对象
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_close(tc_iot_json_writer * w);


/**
 * @brief 开始写入 object 对象
 *
 * @param w writer 对象
 * @param name object 对象名称
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_object_begin(tc_iot_json_writer * w, const char * name);

/**
 * @brief 结束写入 object 对象
 *
 * @param w writer 对象
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_object_end(tc_iot_json_writer * w);

/**
 * @brief 获得 writer 缓存区地址
 *
 * @param w writer 对象
 *
 * @return buffer 地址
 */
char * tc_iot_json_writer_buffer(tc_iot_json_writer * w);

/**
 * @brief 写入字符串类型数据
 *
 * @param w writer 对象
 * @param name 名称
 * @param value 取值
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_string(tc_iot_json_writer * w, const char * name, const char * value);

/**
 * @brief 写入符号整型数据
 *
 * @param w writer 对象
 * @param name 名称
 * @param value 取值
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_int(tc_iot_json_writer * w, const char * name, int value);

/**
 * @brief 写入无符号整型数据
 *
 * @param w writer 对象
 * @param name 名称
 * @param value 取值
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_uint(tc_iot_json_writer * w, const char * name, unsigned int value);

/**
 * @brief 写入浮点数据
 *
 * @param w writer 对象
 * @param name 名称
 * @param value 取值
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_decimal(tc_iot_json_writer * w, const char * name, double value);

/**
 * @brief 写入布尔类型数据
 *
 * @param w writer 对象
 * @param name 名称
 * @param value 取值
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_bool(tc_iot_json_writer * w, const char * name, bool value);

/**
 * @brief 写入 null 数据
 *
 * @param w writer 对象
 * @param name 名称
 * @param value 取值
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_null(tc_iot_json_writer * w, const char * name);

/**
 * @brief 写入已格式化的有效 json 对象数据，对 value 不做任何转义
 *
 * @param w writer 对象
 * @param name 名称
 * @param value 取值
 *
 * @return 结果返回码，> 0 时 为本次调用使用的buffer长度。
 * @see tc_iot_sys_code_e
 */
int tc_iot_json_writer_raw_data(tc_iot_json_writer * w, const char * name, const char * value);

#endif /* end of include guard */
