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

/**
 *  @brief tc_iot_report_firm
     上报设备系统信息
 @par
 上报固件信息，固件信息 key 和 value 由根据实际应用场景指定。例如，要上报 固件版本，sdk 版本，
 硬件版本，调用方式为：

 @code{.c}
 tc_iot_report_firm("firm_version":"1.0.192", "sdk_version":"1.8", "harderwaer_ver":"gprs.v.1.0.2018092", NULL)
 @endcode

 *  @param  p_shadow_client 设备影子对象
 *  @param va_list 可变参数列表，根据实际上报情况指定，格式为 key1,value1,key2,value2, ..., NULL
 * 按照 key value 对方式，依次指定，最后一个参数必须为NULL，作为变参终止符。
 *  @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_report_firm(tc_iot_shadow_client* p_shadow_client, ...);

#endif /* end of include guard */
