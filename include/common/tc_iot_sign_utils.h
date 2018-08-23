#ifndef TC_IOT_SIGN_UTILS_H
#define TC_IOT_SIGN_UTILS_H


/**
 * @brief tc_iot_calc_sign 签名计算基础函数，采用的算法统一为 Sha256 。
 *
 * @param output 签名结果，原始二进制，使用时可以根据实际情况，编码为 Hex 或
 * Base64 等文本格式。
 * @param output_len 签名结果
 * @param secret 签名密钥
 * @param format 签名格式字符串，类似 printf 函数，支持 %s,%d,%x,%c
 * 等格式化占位符
 * @param ... 动态参数列表，根据 format 字段的情况，动态携带对应类型入参。
 *
 * @return 
 */
int tc_iot_calc_sign(unsigned char * output, int output_len, const char * secret, const char * format, ...) ;

/**
 * @brief tc_iot_calc_active_device_sign 计算 HTTP 动态激活请求鉴权签名
 *
 * @param sign_out 签名数据结果输出，签名输出为签名数据经 Base64 及 UrlEncode
 * 编码后结果
 * @param max_sign_len 签名数据结果最大输出长度
 * @param secret 签名密钥，一般为设备的 Device Secret 。
 * @param client_id 设备的 Client Id，一般为 ProductKey@DeivceName 格式。
 * @param device_name 设备名称，即 Device Name 。
 * @param nonce 随机数
 * @param product_id 产品 Id 。
 * @param timestamp Unix 时间戳。
 *
 * @return 
 */
int tc_iot_calc_active_device_sign(char* sign_out, int max_sign_len, 
                            const char* product_secret,
                            const char* device_name,  
                            const char* product_id,
                            long nonce, 
                            long timestamp);


/**
 * @brief tc_iot_calc_auth_sign 计算 HTTP 动态 Token 请求鉴权签名
 *
 * @param sign_out 签名数据结果输出，签名输出为签名数据经 Base64 及 UrlEncode
 * 编码后结果
 * @param max_sign_len 签名数据结果最大输出长度
 * @param secret 签名密钥，一般为设备的 Device Secret 。
 * @param client_id 设备的 Client Id，一般为 ProductKey@DeivceName 格式。
 * @param device_name 设备名称，即 Device Name 。
 * @param nonce 随机数
 * @param product_id 产品 Id 。
 * @param timestamp Unix 时间戳。
 *
 * @return 
 */
int tc_iot_calc_auth_sign(char* sign_out, int max_sign_len, const char* secret, const char* client_id, const char* device_name,
                          long expire, long nonce,
                          const char* product_id,
                          long timestamp);

/**
 * @brief tc_iot_calc_mqtt_dynamic_sign 计算鉴权签名
 *
 * @param sign_out 签名数据结果输输出，签名输出为签名数据经 Base64
 * 编码后的结果
 * @param max_sign_len 签名数据结果最大输出长度
 * @param secret 签名密钥，一般为设备的 Device Secret 。
 * @param client_id 设备的 Client Id，一般为 ProductKey@DeivceName 格式。
 * @param device_name 设备名称，即 Device Name 。
 * @param nonce 随机数
 * @param product_id 产品 Id 。
 * @param timestamp Unix 时间戳。
 *
 * @return 
 */
int tc_iot_calc_mqtt_dynamic_sign(char* sign_out, int max_sign_len, 
        const char* secret, const char* client_id, const char* device_name,
        long nonce,
        const char* product_id,
        long timestamp);

#endif /* end of include guard */
