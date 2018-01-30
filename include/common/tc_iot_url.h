#ifndef TC_IOT_URL_H
#define TC_IOT_URL_H


/**
 * @brief tc_iot_url_encode 对数据进行 url encode ，以便作为
 * query string 或者 url encoded form 请求参数发送给服务端。
 *
 * @param input  待编码数据
 * @param input_len 待编码数据长度
 * @param output 输出缓存区
 * @param max_output_len 输出缓存区最大大小
 *
 * @return 实际输出大小
 */
int tc_iot_url_encode(const char* input, int input_len, char* output,
                      int max_output_len);

/**
 * @brief tc_iot_url_decode 对数据进行 url decode 解码。
 *
 * @param input  待解码码数据
 * @param input_len 待解码数据长度
 * @param output 输出缓存区
 * @param max_output_len 输出缓存区最大大小
 *
 * @return 实际输出大小
 */
int tc_iot_url_decode(const char* input, int input_len, char* output,
                      int max_output_len);

#define HTTP_DEFAULT_PORT 80
#define HTTPS_DEFAULT_PORT 443

#define HTTP_SCHEME_PREFIX "http"
#define HTTP_SCHEME_PREFIX_LEN (sizeof(HTTP_SCHEME_PREFIX) - 1)
#define HTTPS_SCHEME_PREFIX "https"
#define HTTPS_SCHEME_PREFIX_LEN (sizeof(HTTPS_SCHEME_PREFIX) - 1)
#define SCHEME_SPLIT_STR "://"
#define SCHEME_SPLIT_STR_LEN (sizeof(SCHEME_SPLIT_STR) - 1)

/* length of: http://a */
#define URL_LEAST_LEN (HTTP_SCHEME_PREFIX_LEN + HTTPS_SCHEME_PREFIX_LEN + 1)


/**
 * @brief url 地址解析结果
 */
typedef struct _tc_iot_url_parse_result_t {
    int scheme_len; /**< 协议类型长度*/
    int host_start; /**< 服务器地址起始偏移*/
    int host_len; /**< 服务器地址长度*/
    uint16_t port; /**<  服务器端口*/
    char over_tls; /**< 是否是 HTTPS 地址*/
    int path_start; /**< 请求服务路径起始偏移*/
    int path_len; /**< 请求服务路径长度*/
} tc_iot_url_parse_result_t;


/**
 * @brief tc_iot_url_parse 解析 http/https 前缀的 url 地址，
 * 解析服务器地址、端口、请求路径等信息。
 *
 * @param input_url 待解析的 url 地址
 * @param input_url_len 待解析的地址长度
 * @param result 解析结果
 *
 * @return >=0 编码结果长度，<0 失败错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_url_parse(const char* input_url, int input_url_len,
                     tc_iot_url_parse_result_t* result);

#endif /* end of include guard */
