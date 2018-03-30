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
 * @brief tc_iot_mqtt_client_yield 在当前线程为底层服务，让出一定 CPU 执
 * 行时间
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
 * @brief tc_iot_shadow_construct 构造设备影子对象
 *
 * @param p_shadow_client 设备影子对象
 * @param p_config 初始化设备影子对象所需参数配置
 *
 * @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_construct(tc_iot_shadow_client * p_shadow_client,
                            tc_iot_shadow_config *p_config);


/**
 * @brief tc_iot_shadow_destroy 关闭 Shadow client 连接，并销毁 Shadow client
 *
 * @param p_shadow_client 设备影子对象
 */
void tc_iot_shadow_destroy(tc_iot_shadow_client *p_shadow_client);


/**
 * @brief tc_iot_shadow_isconnected 判断设备影子对象，是否已成功连接服务器
 *
 * @param p_shadow_client 设备影子对象
 *
 * @return 1 表示已连接，0 表示未连接。
 */
char tc_iot_shadow_isconnected(tc_iot_shadow_client *p_shadow_client);

/**
 * @brief tc_iot_shadow_yield 在当前线程为底层服务，让出一定 CPU 执
 * 行时间
 *
 * @param  p_shadow_client 设备影子对象
 * @param timeout_ms 等待时延，单位毫秒
 *
 * @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_yield(tc_iot_shadow_client *p_shadow_client, int timeout_ms);


/**
 * @brief tc_iot_shadow_get 异步方式获取设备影子文档
 *
 * @param c 设备影子对象
 * @param buffer 设备影子文档缓存
 * @param buffer_len 设备影子文档缓存最大长度
 * @param callback 请求响应数据回调，可选，传 NULL 则表示不指定响应回调，未指定
 * 回调时，服务端响应，则由 shadow 的默认回调函数 on_receive_msg 处理。
 * @param timeout_ms 请求最大等待时延，可选，当指定 callback 参数时，需指定该回调最大
 * 等待时间。
 * @param session_context 请求相关 context，可选，无需透传时，可传默认的 NULL 。
 *
 * @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_get(tc_iot_shadow_client *c, char * buffer, int buffer_len,  
         message_ack_handler callback, int timeout_ms, void * context);

/**
 * @brief tc_iot_shadow_update 异步方式更新设备影子文档
 *
 * @param c 设备影子对象
 * @param buffer 设备影子文档缓存
 * @param buffer_len 设备影子文档缓存最大长度
 * @param reported reported 字段上报数据，可传递三类数据：
 * 1. NULL : 当不需要上报 reported 字段时，传 NULL 。
 * 2. TC_IOT_JSON_NULL : 当需要清空 reported 字段数据时，传 TC_IOT_JSON_NULL 。 
 * 3. {"a":1,"b":"some string"} : 当需要正常上报 reported 数据时，传有效的 json 字符串。
 *
 * @param desired desired 字段上报数据，可传递三类数据：
 * 1. NULL : 当不需要上报 desired 字段时，传 NULL 。
 * 2. TC_IOT_JSON_NULL : 当需要清空 desired 字段数据时，传 TC_IOT_JSON_NULL 。 
 * 3. {"a":1,"b":"some string"} : 当需要正常上报 desired 数据时，传有效的 json 字符串。
 *
 * @param callback 请求响应数据回调，可选，传 NULL 则表示不指定响应回调，未指定
 * 回调时，服务端响应，则由 shadow 的默认回调函数 on_receive_msg 处理。
 * @param timeout_ms 请求最大等待时延，可选，当指定 callback 参数时，需指定该回调最大
 * 等待时间。
 * @param session_context 请求相关 context，可选，无需透传时，可传默认的 NULL 。
 *
 * @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_update(tc_iot_shadow_client *c, char * buffer, int buffer_len, 
        const char * reported, const char * desired,
        message_ack_handler callback, int timeout_ms, void * session_context);


/**
 * @brief tc_iot_shadow_delete 删除设备属性。
 *
 * @param c 设备影子对象
 * @param buffer 设备影子文档缓存
 * @param buffer_len 设备影子文档缓存最大长度
 * @param reported reported 字段上报数据，可传递三类数据：
 * 1. NULL : 当不需要上报 reported 字段时，传 NULL 。
 * 2. TC_IOT_JSON_NULL : 当需要清空 reported 字段数据时，传 TC_IOT_JSON_NULL 。 
 * 3. {"a":1,"b":"some string"} : 当需要正常上报 reported 数据时，传有效的 json 字符串。
 *
 * @param desired desired 字段上报数据，可传递三类数据：
 * 1. NULL : 当不需要上报 desired 字段时，传 NULL 。
 * 2. TC_IOT_JSON_NULL : 当需要清空 desired 字段数据时，传 TC_IOT_JSON_NULL 。 
 * 3. {"a":1,"b":"some string"} : 当需要正常上报 desired 数据时，传有效的 json 字符串。
 *
 * @param callback 请求响应数据回调，可选，传 NULL 则表示不指定响应回调，未指定
 * 回调时，服务端响应，则由 shadow 的默认回调函数 on_receive_msg 处理。
 * @param timeout_ms 请求最大等待时延，可选，当指定 callback 参数时，需指定该回调最大
 * 等待时间。
 * @param session_context 请求相关 context，可选，无需透传时，可传默认的 NULL 。
 *
 *
 * @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_delete(tc_iot_shadow_client *c, char * buffer, int buffer_len, 
        const char * reported, const char * desired,
        message_ack_handler callback, int timeout_ms, void * session_context);

#endif /* end of include guard */
