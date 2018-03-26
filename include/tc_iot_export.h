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
 *  @param  p_client_config 服务配置参数。
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */
int tc_iot_server_init(tc_iot_shadow_config * p_client_config);


/**
 *  @brief tc_iot_server_loop
 *  服务任务主循环函数，接收服务推送及响应数据。
 *  @param yield_timeout 循环等待时间，单位毫秒
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */

int tc_iot_server_loop(int yield_timeout);

/**
 *  @brief tc_iot_server_destroy
 * 数据点服务析构处理，释放资源。
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */

int tc_iot_server_destroy(void);



/**
 *  @brief tc_iot_report_propeties

    @par
    上报设备数据点参数最新数据状态，更新到服务端。

    @par
    例如，设备定义了 switch、color、brightness，三个参数，上报调用如下：
    @code{.c}
    tc_iot_report_propeties(3, TC_IOT_PROP_switch, &switch, TC_IOT_PROP_color, &color, TC_IOT_PROP_brightness, &brightness);
    @endcode

    @par
    只上报 switch 状态：
    @code{.c}
    tc_iot_report_propeties(1, TC_IOT_PROP_switch, &switch);
    @endcode

    @par
    上报 color 和 brightnes 状态：
    @code{.c}
    tc_iot_report_propeties(2, TC_IOT_PROP_color, &color, TC_IOT_PROP_brightness, &brightness);
    @endcode

 *  @param property_count 上报数据点数
 *  @param va_list 可变参数列表，根据实际上报情况指定，格式为 proprty_id1,value1_address, proprty_id2,value2_address,  ...
 * 按照 参数编号&参数地址 成对方式，依次指定，property_count 的值为 property_id1 ~ property_idN 的总key数。
 *  @return 结果返回码 
 *  @see tc_iot_sys_code_e
 */
int tc_iot_report_propeties(int property_count, ...);

/**
 *  @brief tc_iot_set_control_propeties

    @par
    根据设备控制端要求，发送设备数据点参数控制指令，更新到服务端，推送给设备。

    @par
    例如，设备定义了 switch、color、brightness，三个参数，控制指令调用如下：
    @code{.c}
    tc_iot_set_control_propeties(3, TC_IOT_PROP_switch, &switch, TC_IOT_PROP_color, &color, TC_IOT_PROP_brightness, &brightness);
    @endcode

    @par
    只上报 switch 状态：
    @code{.c}
    tc_iot_set_control_propeties(1, TC_IOT_PROP_switch, &switch);
    @endcode

    @par
    上报 color 和 brightnes 状态：
    @code{.c}
    tc_iot_set_control_propeties(2, TC_IOT_PROP_color, &color, TC_IOT_PROP_brightness, &brightness);
    @endcode
 *
 *  @param property_count 控制指令包含的数据点数
 *  @param va_list 可变参数列表，根据实际上报情况指定，格式为 proprty_id1,value1_address, proprty_id2,value2_address,  ...
 * 按照 参数编号&参数地址 成对方式，依次指定，property_count 的值为 property_id1 ~ property_idN 的总key数。
 *  @return 结果返回码 
 *  @see tc_iot_sys_code_e
 */
int tc_iot_set_control_propeties(int property_count, ...); 

/**
 *  @brief tc_iot_report_firm
     上报设备系统信息
 @par
 上报固件信息，固件信息 key 和 value 由根据实际应用场景指定。例如，要上报 固件版本，sdk 版本，
 硬件版本，调用方式为：

 @code{.c}
 tc_iot_report_firm(3, "firm_version":"1.0.192", "sdk_version":"1.8", "harderwaer_ver":"gprs.v.1.0.2018092")
 @endcode

 *  @param info_count 固件信息数
 *  @param va_list 可变参数列表，根据实际上报情况指定，格式为 key1,value1,key2,value2, ...
 * 按照 key value 对方式，依次指定，info_count 的值为 key1 ~ keyN 的总key数。
 *  @return 结果返回码 
 * @see tc_iot_sys_code_e
 */
int tc_iot_report_firm(int info_count, ...);

#endif /* end of include guard */
