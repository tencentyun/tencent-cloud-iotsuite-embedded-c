#ifndef TC_IOT_EXPORT_H
#define TC_IOT_EXPORT_H

#include "tc_iot_inc.h"

/**
 * @brief tc_iot_set_log_level 设置日志最低输出级别
 *
 * @param log_level 需要设置的日志级别
 */
void tc_iot_set_log_level(tc_iot_log_level_e log_level);

/**
 * @brief tc_iot_get_log_level 获取日志最低输出级别
 *
 * @return 返回当前系统设置的日志级别
 */
tc_iot_log_level_e tc_iot_get_log_level();


/**
 * @brief tc_iot_log_level_enabled 判断指定等级的日志，当前是否开启输出
 *
 * @param log_level 需要判断的日志级别
 *
 * @return 为 1 表示允许输出， 0 表示指定的日志等级低于设置，不允许输出
 */
char tc_iot_log_level_enabled(tc_iot_log_level_e log_level);


#if defined(ENABLE_MQTT)
/**
 * @brief tc_iot_mqtt_refresh_dynamic_sign 刷新签名直连 
 * username 和 password 数据。
 *
 * @param timestamp 时间戳
 * @param nonce 随机值
 * @param p_device_info 设备信息，请求成功后，会自动更新该对象的 username 和
 *  password 成员数据
 * @param reserved 保留参数，填 0 即可
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_refresh_dynamic_sign(long timestamp, long nonce, tc_iot_device_info* p_device_info, long reserved);

/**
 * @brief tc_iot_mqtt_client_construct 构造 MQTT client，并连接MQ服务器
 *
 * @param p_mqtt_client MQTT client 对象，出参。
 * @param p_client_config 用来初始化 MQTT Client 对象的配置信息, 入参。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_construct(tc_iot_mqtt_client* p_mqtt_client,
                                 tc_iot_mqtt_client_config* p_client_config);


/**
 * @brief tc_iot_mqtt_client_destroy 关闭 MQTT client 连接，并销毁 MQTT client
 *
 * @param p_mqtt_client MQTT client 对象
 */
void tc_iot_mqtt_client_destroy(tc_iot_mqtt_client* p_mqtt_client);


/**
 * @brief tc_iot_mqtt_client_is_connected 判断 MQTT client 对象当前是否已连接服
 * 务端
 *
 * @param p_mqtt_client MQTT client 对象
 *
 * @return 1 表示已连接，0 表示未连接。
 */
char tc_iot_mqtt_client_is_connected(tc_iot_mqtt_client* p_mqtt_client);


/**
 * @brief tc_iot_mqtt_client_yield MQTT client
 * 主循环，包含心跳维持、上行消息响应超时检测、服务器下行消息收取等操作。
 *
 * @param p_mqtt_client MQTT client 对象
 * @param timeout_ms 等待时延，单位毫秒
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_yield(tc_iot_mqtt_client* p_mqtt_client, int timeout_ms);


/**
 * @brief tc_iot_mqtt_client_publish 向指定的 Topic 发布消息
 *
 * @param p_mqtt_client MQTT client 对象
 * @param topic Topic 名称
 * @param msg 待发送消息
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_publish(tc_iot_mqtt_client* p_mqtt_client,
                               const char* topic, tc_iot_mqtt_message* msg);


/**
 * @brief tc_iot_mqtt_client_subscribe 订阅指定一个或多个 Topic 的消息
 *
 * @param p_mqtt_client MQTT client 对象
 * @param topic_filter 待订阅 Topic 名称
 * @param qos 本次订阅的 QOS 等级
 * @param msg_handler 订阅消息回调
 * @param context 订阅响应回调 context
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_subscribe(tc_iot_mqtt_client* p_mqtt_client,
                                 const char* topic_filter,
                                 tc_iot_mqtt_qos_e qos,
                                 message_handler msg_handler,
                                 void * context);

/**
 * @brief tc_iot_mqtt_client_unsubscribe 取消对某个或多个 Topic 订阅
 *
 * @param p_mqtt_client MQTT client 对象
 * @param topic_filter 待取消订阅 Topic 名称
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_unsubscribe(tc_iot_mqtt_client* p_mqtt_client,
                                   const char* topic_filter);


/**
 * @brief tc_iot_mqtt_client_disconnect 断开 MQTT client 与服务端的连接
 *
 * @param p_mqtt_client MQTT client 对象
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_disconnect(tc_iot_mqtt_client* p_mqtt_client);

/**
 *  @brief tc_iot_server_init
 * 根据设备配置参数，初始化服务。
 *  @param  p_shadow_client 设备服务对象
 *  @param  p_client_config 服务配置参数。
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */
int tc_iot_server_init(tc_iot_shadow_client* p_shadow_client, tc_iot_shadow_config * p_client_config);


/**
 *  @brief tc_iot_server_loop
 *  服务任务主循环函数，接收服务推送及响应数据。
 *  @param  p_shadow_client 设备服务对象
 *  @param yield_timeout 循环等待时间，单位毫秒
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */

int tc_iot_server_loop(tc_iot_shadow_client* p_shadow_client, int yield_timeout);

/**
 *  @brief tc_iot_server_destroy
 * 数据模板服务析构处理，释放资源。
 *  @param  p_shadow_client 设备影子对象
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */

int tc_iot_server_destroy(tc_iot_shadow_client* p_shadow_client);



/**
 *  @brief tc_iot_report_device_data

    @par
    检查本地变量数据和服务端差异，上报设备数据模板参数最新数据状态，更新到服务端。

 *  @param  p_shadow_client 设备影子对象
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */
int tc_iot_report_device_data(tc_iot_shadow_client* p_shadow_client);

/**
 *  @brief tc_iot_confirm_devcie_data

    @par
    确认服务端控制指令执行结果，如果服务端下发的 desired 指令执行成功，
    则上报最新状态，并清空对应的 desired 指令。
 *  @param  p_shadow_client 设备影子对象
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */
int tc_iot_confirm_devcie_data(tc_iot_shadow_client* p_shadow_client);

#endif

#if defined(ENABLE_COAP)
/**
 * @brief tc_iot_coap_construct 初始化 CoAP 客户端数据
 *
 * @param c 待初始化的 CoAP 客户端数据结构。
 * @param p_client_config 初始化相关参数，包括
 * CoAP 服务地址及端口、是否使用 DTLS及DTLS PSK、
 * 产品信息、设备名称、设备密钥、回调函数等。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_coap_construct(tc_iot_coap_client* c, tc_iot_coap_client_config* p_client_config);


/**
 * @brief tc_iot_coap_auth 发起认证，获取后续服务所需的设备 Token。
 *
 * @param c 已初始化好的 CoAP 客户端。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_coap_auth(tc_iot_coap_client* c);


/**
 * @brief tc_iot_coap_send_message 向服务端发送 CoAP 消息。
 *
 * @param c 已成功获取获取授权 Token 的 CoAP 客户端。
 * @param message 待发送的消息
 * @param callback 消息回调
 * @param timeout_ms 消息最大等待时延
 * @param session_context 消息回调透传参数
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_coap_send_message(tc_iot_coap_client* c, tc_iot_coap_message* message,
        tc_iot_coap_con_handler callback, int timeout_ms, void * session_context);


/**
 * @brief tc_iot_coap_publish 为基于 tc_iot_coap_send_message
 * 的上层逻辑封装，用来单向发送上报消息。
 *
 * @param c 已成功获取获取授权 Token 的 CoAP 客户端。
 * @param uri_path 上报接口 URI Path，当前固定填写
 * TC_IOT_COAP_SERVICE_PUBLISH_PATH。
 * @param topic_query_uri 上报消息发送目的 Topic
 * 参数，参数固定格式为：tp=Topic_Name，即如果上报消息到 TopicA，
 * 参数应增加 tp= 前缀后，填写为 “tp=TopicA”
 * @param msg 上报消息 Payload 。
 * @param callback 上报结果回调。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 *
 */
int tc_iot_coap_publish( tc_iot_coap_client * c, const char * uri_path, 
        const char * topic_query_uri, const char * msg, tc_iot_coap_con_handler callback);

/**
 * @brief tc_iot_coap_rpc 为基于 tc_iot_coap_send_message 
 * 的上层逻辑封装，用来调用影子服务或基于自定义 Topic 的远程服务。
 *
 * @param c 已成功获取获取授权 Token 的 CoAP 客户端。
 * @param uri_path 上报接口 URI Path，当前固定填写
 * TC_IOT_COAP_SERVICE_RPC_PATH。
 * @param topic_query_uri RPC 请求参数发送目的 Topic
 * 参数，参数固定格式为：pt=Topic_Name，即如果到 TopicUpdate，
 * 参数应增加 pt= 前缀后，填写为 “pt=TopicUpdate”
 * @param topic_resp_uri RPC 调用响应 Topic
 * 参数，参数固定格式为：st=Topic_Name，即如果到 TopicCmd，
 * 参数应增加 st= 前缀后，填写为 “st=TopicUpdate”
 * @param msg 上报消息 Payload 。
 * @param callback RPC 调用结果回调。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 *
 */
int tc_iot_coap_rpc( tc_iot_coap_client * c, const char * uri_path, 
        const char * topic_query_uri, const char * topic_resp_uri,
        const char * msg, tc_iot_coap_con_handler callback);

/**
 *
 * @brief tc_iot_coap_yield  CoAP client 主循环，包含上行消息响应超时
 * 检测、服务器下行消息收取等操作。
 *
 * @param c CoAP client 对象
 * @param timeout_ms 等待时延，单位毫秒
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_coap_yield(tc_iot_coap_client * c, int timeout_ms);


/**
 * @brief tc_iot_coap_destroy 释放 CoAP client 对象相关资源。
 *
 * @param c CoAP client 对象
 */
void tc_iot_coap_destroy(tc_iot_coap_client* c);


/**
 * @brief tc_iot_coap_get_message_code 获取消息请求或返回码。
 *
 * @param message CoAP 消息
 *
 * @return 消息请求或返回码
 * @see tc_iot_coap_rsp_code
 */
unsigned char tc_iot_coap_get_message_code(tc_iot_coap_message* message);

/**
 * @brief tc_iot_coap_get_message_payload 获取 CoAP 消息的 Payload 内容。
 *
 * @param message CoAP 消息
 * @param payload_len Payload 的长度
 * @param payload Payload 内容首地址
 *
 * @return 结果返回码，返回 TC_IOT_SUCCESS 则说明获取成功。
 * @see tc_iot_sys_code_e
 */
int tc_iot_coap_get_message_payload(tc_iot_coap_message* message, int *payload_len, unsigned char **payload);

#endif


#if defined(ENABLE_OTA)
/**
 * @brief tc_iot_ota_construct 初始化 OTA 服务对象
 *
 * @param ota_handler 待初始化的 OTA 对象
 * @param mqtt_client OTA 对象通过 MQTT 协议与服务端通讯，需要指定用于通讯的
 * MQTT Client。
 * @param sub_topic OTA 下行消息 Topic
 * @param pub_topic OTA 上行消息 Topic
 * @param ota_msg_callback OTA 下行消息回调通知函数
 *
 * @return 结果返回码，返回 TC_IOT_SUCCESS 则说明获取成功。
 * @see tc_iot_sys_code_e
 */
int tc_iot_ota_construct(tc_iot_ota_handler * ota_handler, tc_iot_mqtt_client * mqtt_client, 
        const char * sub_topic, const char * pub_topic, message_handler ota_msg_callback);


/**
 * @brief tc_iot_ota_destroy OTA 服务析构处理，取消 OTA 消息订阅，释放资源。
 *
 * @param ota_handler 待释放的 OTA 服务对象
 */
void tc_iot_ota_destroy(tc_iot_ota_handler * ota_handler);


/**
 * @brief tc_iot_ota_report_upgrade OTA 升级执行过程中，上报固件下载及升级进度。
 *
 * @param ota_handler OTA 服务对象
 * @param state 当前升级进展枚举
 * @param message 辅助消息
 * 1.当前进展为成功时：上报 TC_IOT_OTA_MESSAGE_SUCCESS ；
 * 2.出错或失败时：上报对应的失败消息或者 TC_IOT_OTA_MESSAGE_FAILED ；
 * @param percent 百分比，0~100，仅当 state 为 OTA_DOWNLOAD
 * 时有效，用来上报下载完成百分比。
 *
 * @return 结果返回码，返回 TC_IOT_SUCCESS 则说明获取成功。
 * @see tc_iot_sys_code_e
 */
int tc_iot_ota_report_upgrade(tc_iot_ota_handler * ota_handler, tc_iot_ota_state_e state, char * message, int percent);

/**
 *  @brief tc_iot_ota_report_firm
     上报设备系统信息
 @par
 上报固件信息，固件信息 key 和 value 由根据实际应用场景指定。例如，要上报 固件版本，sdk 版本，
 硬件版本，调用方式为：

 @code{.c}
 tc_iot_ota_report_firm(&handler,
         "sdk-ver", TC_IOT_SDK_VERSION,
         "firm-ver",TC_IOT_FIRM_VERSION, NULL);
 @endcode

 *  @param ota_handler OTA 对象
 *  @param va_list 可变参数列表，根据实际上报情况指定，格式为 key1,value1,key2,value2, ..., NULL
 * 按照 key value 对方式，依次指定，最后一个参数必须为NULL，作为变参终止符。
 *  @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_ota_report_firm(tc_iot_ota_handler * ota_handler, ...);


/**
 * @brief tc_iot_ota_request_content_length 获取指定 URL 文件的大小。
 *
 * @param api_url 带获取信息的文件 URL
 *
 *  @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_ota_request_content_length(const char* api_url);


/**
 * @brief tc_iot_ota_download 根据指定的固件 URL 地址，下载固件
 *
 * @param api_url 固件地址
 * @param partial_start 下载偏移地址，默认填 0，当需要续传时，传需要偏移的值。
 * @param download_callback
 * 下载过程回调，由于缓存区有限，每下载成功一段数据，就会通过本回调，通知设备端
 * 进行处理，例如，写入到 Flash 中。
 * @param context 回调透传数据
 *
 *  @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_ota_download(const char* api_url, int partial_start, tc_iot_http_download_callback download_callback, const void * context);

#endif


/**
 * @brief  tc_iot_http_mqapi_rpc 通过 HTTP 接口访问数据模板数据。
 *
 * @param result 响应数据缓存
 * @param result_len 响应数据缓存最大长度
 * @param api_url 请求接口地址
 * @param root_ca_path TLS 外部证书路径
 * @param timestamp 请求时间戳
 * @param nonce 随机数
 * @param p_device_info 设备信息
 * @param message 请求消息
 *
 * @return 结果返回码，> 0  时表示result中有效数据长度，<= 0 时为错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_http_mqapi_rpc( char * result, int result_len,
        const char* api_url, char* root_ca_path, long timestamp, long nonce,
        tc_iot_device_info* p_device_info, const char * message
        );

/**
  * @brief tc_iot_refresh_auth_token 鉴权模式为动态令牌模式时，通过
  * 本接口获取访问 MQTT 服务端动态用户名和密码，本接口支持自定义令牌过期时间。
 *
 * @param timestamp 时间戳
 * @param nonce 随机值
 * @param p_device_info 设备信息，请求成功后，会自动更新该对象的 username 和
 *  password 成员数据
 * @param expire token 有效期单位为s，最大有效时间为2个月 = 60*86400 s。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_refresh_auth_token(long timestamp, long nonce, tc_iot_device_info* p_device_info, long expire);

#endif /* end of include guard */
