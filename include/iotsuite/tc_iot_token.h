#ifndef TC_IOT_TOKEN_H
#define TC_IOT_TOKEN_H


/**
 * @brief http_refresh_auth_token 请求 MQTT 访问服务端动态用户名和密码
 *
 * @param api_url 请求接口的地址
 * @param root_ca_path 额外加载证书地址，一般填 NULL 即可
 * @param timestamp 时间戳
 * @param nonce 随机值
 * @param p_device_info 设备信息，请求成功后，会自动更新该对象的 username 和
 *  password 成员数据
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int http_refresh_auth_token(const char* api_url, char* root_ca_path,
                            long timestamp, long nonce,
                            tc_iot_device_info* p_device_info);

#define TC_IOT_TOKEN_MAX_EXPIRE_SECOND    (86400*60)

/**
 * @brief http_refresh_auth_token 请求 MQTT 访问服务端动态用户名和密码
 *
 * @param api_url 请求接口的地址
 * @param root_ca_path 额外加载证书地址，一般填 NULL 即可
 * @param timestamp 时间戳
 * @param nonce 随机值
 * @param p_device_info 设备信息，请求成功后，会自动更新该对象的 username 和
 *  password 成员数据
 * @param expire token 有效期单位为s，最大有效时间为2个月 = 60*86400 s。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int http_refresh_auth_token_with_expire(const char* api_url, char* root_ca_path, long timestamp, long nonce,
        tc_iot_device_info* p_device_info, long expire);

/**
 * @brief http_get_device_secret 请求 MQTT 访问服务端的 device secrect
 *
 * @param api_url 请求接口的地址
 * @param root_ca_path 额外加载证书地址，一般填 NULL 即可
 * @param timestamp 时间戳
 * @param nonce 随机值
 * @param p_device_info 设备信息，请求成功后，会自动更新该对象的 device_secret 成员数据
 * @param expire token 有效期单位为s，最大有效时间为2个月 = 60*86400 s。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int http_get_device_secret(const char* api_url, char* root_ca_path, long timestamp, long nonce,
        tc_iot_device_info* p_device_info);

#endif /* end of include guard */
