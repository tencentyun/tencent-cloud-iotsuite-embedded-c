#ifndef TC_IOT_MQTT_H
#define TC_IOT_MQTT_H

#include "tc_iot_inc.h"


/**
 * @brief 消息 ID 最大取值，MQTT 协议限定 [Packet
 * Identifier](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718025) 只有
 * 2 字节，请保持 65535 这个值不要改动 
 */
#define TC_IOT_MAX_PACKET_ID 65535


/**
 * @brief 客户端最大订阅 Topic 数，请根据业务场景，
 * 实际订阅的最大 Topic 数情况进行配置。
 */
#define TC_IOT_MAX_MESSAGE_HANDLERS 5



/**
 * @brief 消息订阅回包，SUBACK 返回码
 * [returnCode](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718071)
 *
 */
typedef enum _tc_iot_mqtt_qos_e {
    TC_IOT_QOS0 = 0,  /**< QOS0*/
    TC_IOT_QOS1 = 1, /**< QOS1*/
    TC_IOT_QOS2 = 2, /**< QOS2*/
    TC_IOT_SUBFAIL = 0x80, /**< 订阅失败，有可能 Topic 不存在，或者无权限等*/
} tc_iot_mqtt_qos_e;


/**
 * @brief MQTT 连接请求回包，CONNACK 返回码
 * [returnCode](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718035)
 */
typedef enum _tc_iot_mqtt_conn_ack_e {
    TC_IOT_CONN_SUCCESS = 0, /**< 连接成功*/
    TC_IOT_CONN_PROTOCOL_UNACCEPTABLE = 1, /**< 连接失败，不支持请求的协议版本*/
    TC_IOT_CONN_CLIENT_ID_INVALID = 2, /**< 连接失败，Client Id 判定非法*/
    TC_IOT_CONN_SERVICE_UNAVAILABLE = 3,  /**< 连接失败，后端服务不可用*/
    TC_IOT_CONN_BAD_USER_OR_PASSWORD = 4,  /**< 连接失败，用户名或密码校验不通过*/
    TC_IOT_CONN_NOT_AUTHORIZED = 5,  /**< 连接失败，用户访问未授权*/
} tc_iot_mqtt_conn_ack_e;

/**
 * @brief MQTT Publis请求消息，头部参数定义(非协议头)，关键参数说明
 */
typedef struct _tc_iot_mqtt_message {
    int qos; /**< QOS级别*/
    unsigned char retained;  /**< 是否要求服务端持久保存此消息*/
    unsigned char dup; /**< 是否重发消息*/
    unsigned short id; /**< 消息 ID*/
    void* payload; /**< 消息包体内容*/
    size_t payloadlen; /**< 消息包体内容长度*/
} tc_iot_mqtt_message;


/**
 * @brief 接收到的订阅消息
 */
typedef struct _tc_iot_message_data {
    tc_iot_mqtt_message* message; /**< 消息内容*/
    MQTTString* topicName; /**< 所属 Topic*/
    void * mqtt_client;
    void * context;
} tc_iot_message_data;


/**
 * @brief 连接请求响应，CONNACK 回包数据
 */
typedef struct _tc_iot_mqtt_connack_data {
    unsigned char rc;  /**< @link tc_iot_mqtt_conn_ack_e 返回码 @endlink*/
    unsigned char sessionPresent; /**< [Session Present](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718035)*/
} tc_iot_mqtt_connack_data;


/**
 * @brief 订阅消息返回码数据。
 */
typedef struct _tc_iot_mqtt_suback_data {
    tc_iot_mqtt_qos_e grantedQoS; /**< 消息 QOS 等级*/
} tc_iot_mqtt_suback_data;

typedef void (*message_handler)(tc_iot_message_data*);


/**
 * @brief 客户端连接状态
 */
typedef enum _tc_iot_mqtt_client_state_e {
    CLIENT_INVALID, /**< 非法状态，数据未初始化*/
    CLIENT_INTIALIAZED, /**< 已初始化，连接相关信息已生成*/
    CLIENT_NETWORK_READY, /**< 网络层(TCP 或 TLS)连接已完成，可进行 MQTT CONN 请求*/
    CLIENT_CONNECTED, /**< 应用层连接完成：MQTT CONN/CONNACK 成功*/
} tc_iot_mqtt_client_state_e;

typedef struct _tc_iot_mqtt_client tc_iot_mqtt_client;

/**
 * @brief  连接断开回调函数原型，用户可通过实现该函数，
 * 并注册到 MQTT client上，实现对连接状态的监控，以便
 * 在连接断开时，得到通知，做对应处理。
 *
 * @param p_mqtt_client MQTT client 对象
 *
 */
typedef void (*disconnectHandler)(tc_iot_mqtt_client*, void*);



/**
 * @brief MQTT client 对象结构，保存 MQTT 客户端相关配置、连接状态、
 * 回调处理、时延要求、收发缓存区等信息。
 */
struct _tc_iot_mqtt_client {
    unsigned int command_timeout_ms;  /**< MQTT 指令超时配置，根据客户端设备工作环境，实际网络情况进行合理配置。
                                        1) 对于客户端网络不稳定情况下，适当延长时延，可以避免无效重连、重试；
                                        2) 对于客户端网络较为稳定的情况下，可以设置较短的时延，确保当网络或服务异常时，
                                        能及时被超时机制发现，及时重连恢复服务。*/
    size_t buf_size;  /**< 发送缓存区大小，固定为 TC_IOT_CLIENT_SEND_BUF_SIZE*/
    size_t readbuf_size;  /**< 接收缓存区大小，固定为 TC_IOT_CLIENT_READ_BUF_SIZE*/
    unsigned char buf[TC_IOT_CLIENT_SEND_BUF_SIZE]; /**< 发送缓存区，根据业务情况，实际发送最大包的大小，
                                                      合理设定 TC_IOT_CLIENT_READ_BUF_SIZE 的值 */
    unsigned char readbuf[TC_IOT_CLIENT_READ_BUF_SIZE];  /**< 接收缓存区的大小，根据服务端下发订阅消息最大包大小，
                                                           合理设定 TC_IOT_CLIENT_READ_BUF_SIZE 的值 */
    unsigned int keep_alive_interval;  /**< 客户端与服务端心跳包发送间隔，单位是秒 */
    char auto_reconnect; /**< 当检测到超时或网络异常时，是否自动重连 */
    unsigned int reconnect_timeout_ms; /**< 自动重连当前等待时延*/
    int clean_session;  /**< [CleanSession](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030) 状态*/
    unsigned int next_packetid;  /**< 下一个可用的消息 ID*/
    char ping_outstanding; /**< 是否正在进行心跳 PING 操作*/
    tc_iot_mqtt_client_state_e state; /**< 连接状态*/
    MQTTPacket_connectData connect_options; /**< 连接配置 */

    struct MessageHandlers {
        const char* topicFilter;
        tc_iot_mqtt_qos_e qos;
        message_handler fp;
        void * context;
    } message_handlers[TC_IOT_MAX_MESSAGE_HANDLERS]; /**< 订阅消息回调*/

    message_handler  default_msg_handler; /**< 订阅消息默认回调*/
    disconnectHandler disconnect_handler; /**< 连接断开通知回调*/

    tc_iot_network_t ipstack; /**< 网络服务*/
    tc_iot_timer last_sent; /**< 最近一次发包定时器，用来判断是否需要发起 keep alive 心跳*/
    tc_iot_timer last_received; /**< 最近一次收包定时器，用来判断是否需要发起 keep alive 心跳*/
    tc_iot_timer ping_timer; /**< keep alive ping请求定时器*/
    tc_iot_timer reconnect_timer; /**< 重连定时器，用来判断是否需要发起新一轮重连尝试*/
    
    long         client_init_time;
} ;

typedef enum _tc_iot_device_auth_mode_e {
    TC_IOT_MQTT_AUTH_NONE,
    TC_IOT_MQTT_AUTH_STATIC_PASS,
    TC_IOT_MQTT_AUTH_DYNAMIC_TOKEN,
} tc_iot_device_auth_mode_e;

/**
 * @brief 设备信息
 */
typedef struct _tc_iot_device_info {
    char secret[TC_IOT_MAX_SECRET_LEN];  /**< 设备签名秘钥*/
    char product_id[TC_IOT_MAX_PRODUCT_ID_LEN]; /**< 设备 Product Id*/
    char device_name[TC_IOT_MAX_DEVICE_NAME_LEN];  /**< 设备 Device Name*/
    char client_id[TC_IOT_MAX_CLIENT_ID_LEN]; /**< 设备 Client Id*/

    char username[TC_IOT_MAX_USER_NAME_LEN]; /**< 连接 MQ 服务端的 Username*/
    char password[TC_IOT_MAX_PASSWORD_LEN]; /**< 连接 MQ 服务端的 Password*/
    long token_expire_time;  /**< username & password 超时时间*/
} tc_iot_device_info;


/**
 * @brief MQTT client 客户端配置
 */
typedef struct _tc_iot_mqtt_client_config {
    tc_iot_device_info device_info;  /**< 设备信息*/
    char* host; /**< MQ 服务地址*/
    uint16_t port; /**< MQ 服务端口*/
    int command_timeout_ms; /**< MQTT 指令超时时延，单位毫秒*/
    int tls_handshake_timeout_ms; /**< TLS 握手时延，单位毫秒*/
    int keep_alive_interval; /**< 心跳保持间隔，单位秒 */
    char clean_session; /**< Clean Session 标志*/
    char use_tls; /**< 是否通过 TLS 连接服务*/
    char auto_reconnect; /**< 是否自动重连 */
    const char* p_root_ca; /**< 根证书*/
    const char* p_client_crt; /**< 客户端证书*/
    const char* p_client_key; /**< 客户端私钥*/

    disconnectHandler disconnect_handler; /**< 连接断开回调*/
    message_handler   default_msg_handler; /**< 默认消息处理回调*/
    char willFlag;
    MQTTPacket_willOptions will; 
} tc_iot_mqtt_client_config;


int tc_iot_mqtt_init(tc_iot_mqtt_client* c,
                     tc_iot_mqtt_client_config* p_client_config);

int tc_iot_mqtt_connect_with_results(tc_iot_mqtt_client* client,
                                     MQTTPacket_connectData* options,
                                     tc_iot_mqtt_connack_data* data);
int tc_iot_mqtt_connect(tc_iot_mqtt_client* client,
                        MQTTPacket_connectData* options);
int tc_iot_mqtt_reconnect(tc_iot_mqtt_client* c);
int tc_iot_mqtt_publish(tc_iot_mqtt_client* client, const char*,
                        tc_iot_mqtt_message*);
int tc_iot_mqtt_set_message_handler(tc_iot_mqtt_client* c,
                                    const char* topicFilter,
                                    tc_iot_mqtt_qos_e qos,
                                    message_handler message_handler,
                                    void * context);
int tc_iot_mqtt_subscribe(tc_iot_mqtt_client* client, const char* topicFilter,
                          tc_iot_mqtt_qos_e, message_handler, void * context);
int tc_iot_mqtt_subscribe_with_results(tc_iot_mqtt_client* client,
                                       const char* topicFilter,
                                       tc_iot_mqtt_qos_e, message_handler,
                                       void * context,
                                       tc_iot_mqtt_suback_data* data);
int tc_iot_mqtt_unsubscribe(tc_iot_mqtt_client* client,
                            const char* topicFilter);
int tc_iot_mqtt_disconnect(tc_iot_mqtt_client* client);
int tc_iot_mqtt_yield(tc_iot_mqtt_client* client, int time);
int tc_iot_mqtt_is_connected(tc_iot_mqtt_client* client);
int tc_iot_mqtt_set_state(tc_iot_mqtt_client* client,
                          tc_iot_mqtt_client_state_e state);
tc_iot_mqtt_client_state_e tc_iot_mqtt_get_state(tc_iot_mqtt_client* client);
char tc_iot_mqtt_get_auto_reconnect(tc_iot_mqtt_client* client);
int tc_iot_mqtt_set_auto_reconnect(tc_iot_mqtt_client* client,
                                   char auto_reconnect);
void tc_iot_init_mqtt_conn_data(MQTTPacket_connectData * conn_data);
void tc_iot_mqtt_destroy(tc_iot_mqtt_client* c);

#endif /* end of include guard */
