#ifndef IOT_HTTP_UTILS_H
#define IOT_HTTP_UTILS_H

#include "tc_iot_inc.h"

#define HTTP_VER_1_0 "1.0"
#define HTTP_VER_1_1 "1.1"

#define HTTP_PUT "GET"
#define HTTP_POST "POST"
#define HTTP_GET "GET"

#define HTTP_SPLIT_STR "\r\n"
#define HTTP_REQUEST_LINE_FMT ("%s %s HTTP/%s" HTTP_SPLIT_STR)
#define HTTP_HEADER_FMT "%s: %.*s\r\n"
#define HTTP_HEADER_HOST "Host"
#define HTTP_HEADER_ACCEPT "Accept"
#define HTTP_HEADER_ACCEPT_ENCODING "Accept-Encoding"
#define HTTP_HEADER_USER_AGENT "User-Agent"
#define HTTP_HEADER_CONTENT_LENGTH "Content-Length"
#define HTTP_HEADER_CONTENT_TYPE "Content-Type"
#define HTTP_CONTENT_FORM_URLENCODED "application/x-www-form-urlencoded"
#define HTTP_CONTENT_FORM_DATA "multipart/form-data"
#define HTTP_CONTENT_JSON "application/json"

/* examples: */
/* HTTP/1.0 200 OK */
/* HTTP/1.1 404 Not Found */
/* HTTP/1.1 503 Service Unavailable */
#define HTTP_RESPONSE_STATE_PREFIX "HTTP/1."
#define HTTP_RESPONSE_STATE_PREFIX_LEN (sizeof(HTTP_RESPONSE_STATE_PREFIX)-1)


#define HTTP_BODY_FMT "\r\n%s"

typedef struct _tc_iot_http_request {
    tc_iot_yabuffer_t buf;
} tc_iot_http_request;



/**
 * @brief tc_iot_http_request_init 初始化请求缓存区
 *
 * @param request HTTP 请求对象
 * @param method 请求方法：GET/POST/PUT ...
 * @param abs_path 请求地址："/", "/index", "/news?t=abc" ...
 * @param abs_path_len 请求地址长度
 * @param http_version HTTP 协议版本：1.0, 1.1 ...
 *
 * @return >=0 初始化后数据占用缓存长度，<0 返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_http_request_init(tc_iot_http_request* request, const char* method,
                             const char* abs_path, int abs_path_len,
                             const char* http_version);

/**
 * @brief tc_iot_http_request_append_header 添加 HTTP header头部信息。
 *
 * @param request HTTP 请求对象
 * @param header Header 名称
 * @param val Header 值
 *
 * @return >=0 添加的 Header 实际占用空间大小，<0 返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_http_request_append_header(tc_iot_http_request* request,
                                      const char* header, const char* val);

/**
 * @brief tc_iot_http_request_n_append_header 添加 HTTP header头部信息。
 *
 * @param request HTTP 请求对象
 * @param header Header 名称
 * @param val Header 值
 * @param val_len Header 值的长度
 *
 * @return >=0 添加的 Header 实际占用空间大小，<0 返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_http_request_n_append_header(tc_iot_http_request* request,
                                        const char* header, const char* val,
                                        int val_len);

/**
 * @brief tc_iot_http_request_append_body 添加请求包体
 *
 * @param request HTTP 请求对象
 * @param body 包体数据
 *
 * @return >=0 添加的 Body 实际占用空间大小，<0 返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_http_request_append_body(tc_iot_http_request* request,
                                    const char* body);


/**
 * @brief tc_iot_create_http_request 创建 HTTP 请求
 *
 * @param request HTTP 请求对象
 * @param host 服务器地址
 * @param host_len 服务器地址长度
 * @param method 请求方法
 * @param abs_path 请求服务路径
 * @param abs_path_len 请求服务路径长度
 * @param http_version HTTP 协议版本
 * @param user_agent User Agent 头部信息
 * @param content_type Content Type 头部信息
 * @param body HTTP 请求Body
 *
 * @return 返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_create_http_request(tc_iot_http_request* request, const char* host,
                               int host_len, const char* method,
                               const char* abs_path, int abs_path_len,
                               const char* http_version, const char* user_agent,
                               const char* content_type, const char* body);

/**
 * @brief tc_iot_create_post_request 创建 HTTP POST 请求
 *
 * @param request HTTP 请求对象
 * @param abs_path 请求服务路径
 * @param abs_path_len 请求服务路径长度
 * @param host 服务器地址
 * @param host_len 服务器地址长度
 * @param body HTTP 请求Body
 *
 * @return 返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_create_post_request(tc_iot_http_request* request,
                               const char* abs_path, int abs_path_len,
                               const char* host, int host_len,
                               const char* body);


/**
 * @brief tc_iot_calc_auth_sign 计算 Token 请求签名
 *
 * @param sign_out 签名( Base64 编码)结果
 * @param max_sign_len 签名( Bsse64 编码)结果区长度
 * @param secret 签名密钥
 * @param secret_len 签名密钥长度
 * @param client_id Client Id
 * @param client_id_len Client Id 长度
 * @param device_name Device Name
 * @param device_name_len Device Name 长度
 * @param expire Token有效期
 * @param nonce 随机数
 * @param product_id Product Id
 * @param product_id_len Product Id 长度
 * @param timestamp 时间戳
 *
 * @return >=0 签名结果实际长度，<0 错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_calc_auth_sign(char* sign_out, int max_sign_len, const char* secret,
                          int secret_len, const char* client_id,
                          int client_id_len, const char* device_name,
                          int device_name_len, long expire, long nonce,
                          const char* product_id, int product_id_len,
                          long timestamp);


/**
 * @brief tc_iot_create_auth_request_form 构造 Token HTTP 签名请求 form
 *
 * @param form 结果缓存区
 * @param max_form_len 结果缓存区最大大小
 * @param secret 签名密钥
 * @param secret_len 签名密钥长度
 * @param client_id Client Id
 * @param client_id_len Client Id 长度
 * @param device_name Device Name
 * @param device_name_len Device Name 长度
 * @param expire Token有效期
 * @param nonce 随机数
 * @param product_id Product Id
 * @param product_id_len Product Id 长度
 * @param timestamp 时间戳
 *
 * @return >=0 签名结果实际长度，<0 错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_create_auth_request_form(char* form, int max_form_len,
                                    const char* secret, int secret_len,
                                    const char* client_id, int client_id_len,
                                    const char* device_name,
                                    int device_name_len, long expire,
                                    long nonce, const char* product_id,
                                    int product_id_len, long timestamp);

/**
 * @brief tc_iot_create_active_device_form 构造 get device 设备激活 HTTP 签名请求 form
 *
 * @param form 结果缓存区
 * @param max_form_len 结果缓存区最大大小
 * @param secret 签名密钥, 请使用控制台的 product password
 * @param secret_len 签名密钥长度
 * @param device_name Device Name
 * @param device_name_len Device Name 长度
 * @param product_id Product Id , 例子 : "iot-dalqbv1g"	
 * @param product_id_len Product Id 长度
 * @param nonce 随机数
 * @param timestamp 时间戳 *
 * @return >=0 签名结果实际长度，<0 错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_create_active_device_form(char* form, int max_form_len,
									const char* product_secret, int secret_len,
                                    const char* device_name, int device_name_len, 
									const char* product_id,int product_id_len,
                                    long nonce, long timestamp);

/**
 * @brief tc_iot_parse_http_response_code 解析 HTTP 响应数据返回码
 *
 * @param resp HTTP 响应数据字符串
 *
 * @return >0 HTTP Status Code, 比如 200、404、50x 等， <0 数据格式非法，无法解
 * 析
 */
int tc_iot_parse_http_response_code(const char * http_resp);

#endif /* end of include guard */
