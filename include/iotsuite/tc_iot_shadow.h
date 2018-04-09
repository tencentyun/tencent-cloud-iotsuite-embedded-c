#ifndef TC_IOT_SHADOW_H
#define TC_IOT_SHADOW_H

#define TC_IOT_MAX_PROPERTY_COUNT   128
#define TC_IOT_MAX_FIRM_INFO_COUNT  5

typedef double tc_iot_shadow_number;
typedef int tc_iot_shadow_int;
typedef int tc_iot_shadow_enum;
typedef bool tc_iot_shadow_bool;

typedef enum _tc_iot_shadow_data_type_e {
    TC_IOT_SHADOW_TYPE_INVALID = 0,
    TC_IOT_SHADOW_TYPE_BOOL = 1,
    TC_IOT_SHADOW_TYPE_NUMBER = 2,
    TC_IOT_SHADOW_TYPE_ENUM = 3,
    TC_IOT_SHADOW_TYPE_INT = 4,
} tc_iot_shadow_data_type_e;

typedef struct _tc_iot_shadow_property_def {
    const char * name;
    int id;
    tc_iot_shadow_data_type_e  type;
    int offset;
} tc_iot_shadow_property_def;


/**
 * @brief 影子设备配置
 */
typedef struct _tc_iot_shadow_config {
    tc_iot_mqtt_client_config mqtt_client_config;  /**< MQTT 相关配置*/
    char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN]; /**< 影子设备订阅 Topic*/
    char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];  /**< 影子设备消息 Publish Topic*/
    message_handler on_receive_msg; /**< 影子设备消息回调*/
    int property_total;
    tc_iot_shadow_property_def * properties;
    tc_iot_event_handler event_notify;
    void * p_current_device_data;
    void * p_reported_device_data;
    void * p_desired_device_data;
} tc_iot_shadow_config;

typedef enum _tc_iot_command_ack_status_e {
    TC_IOT_ACK_SUCCESS,
    TC_IOT_ACK_TIMEOUT,
} tc_iot_command_ack_status_e;

typedef void (*message_ack_handler)(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context);


#define TC_IOT_SESSION_ID_LEN     8
#define TC_IOT_MAX_SESSION_COUNT  3

typedef struct _tc_iot_shadow_session{
    char sid[TC_IOT_SESSION_ID_LEN+1];
    tc_iot_timer        timer;
    message_ack_handler handler;
    void * session_context;
}tc_iot_shadow_session;


/**
 * @brief 影子设备客户端
 */
typedef struct _tc_iot_shadow_client {
    tc_iot_shadow_config* p_shadow_config; /**< 影子设备配置*/
    tc_iot_mqtt_client mqtt_client; /**< MQTT 客户端*/
    tc_iot_shadow_session sessions[TC_IOT_MAX_SESSION_COUNT];
    char desired_bits[(TC_IOT_MAX_PROPERTY_COUNT/8)+1];
    char reported_bits[(TC_IOT_MAX_PROPERTY_COUNT/8)+1];
} tc_iot_shadow_client;


/**
 * @brief tc_iot_shadow_doc_pack_for_get 构造 get 请求包，用于读取影子设备服务端数据。
 *
 * @param buffer 输出结果缓存区
 * @param buffer_len 输出结果缓存区大小
 * @param c 影子设备客户端
 *
 * @return 输出结果长度
 */
int tc_iot_shadow_doc_pack_for_get(char * buffer, int buffer_len, tc_iot_shadow_client *c);


/**
 * @brief tc_iot_shadow_doc_pack_for_update 生成影子设备文档或属性上报请求包
 *
 * @param buffer 输出结果缓存区
 * @param buffer_len 输出结果缓存区大小
 * @param c 影子设备客户端
 * @param reported Reported 文档上报数据
 * @param desired  Desired 文档上报数据
 *
 * @return 输出结果长度
 */
int tc_iot_shadow_doc_pack_for_update(char * buffer, int buffer_len, tc_iot_shadow_client *c, const char * reported, const char * desired);

/**
 * @brief tc_iot_shadow_doc_pack_for_delete 生成影子设备删除文档或属性请求包
 *
 * @param buffer 输出结果缓存区
 * @param buffer_len 输出结果缓存区大小
 * @param c 影子设备客户端
 * @param reported Reported 属性数据删除指令
 * @param desired  Desired 属性数据删除指令
 *
 * @return 输出结果长度
 */
int tc_iot_shadow_doc_pack_for_delete(char * buffer, int buffer_len, tc_iot_shadow_client *c, const char * reported, const char * desired);


/*--- begin 影子设备请求响应包 method 字段取值----*/
/* 请求类 */
/**< 读取服务端影子设备数据
 * {"method":"get"}
 * */
#define TC_IOT_MQTT_METHOD_GET       "get"
/**< 更新服务端影子设备数据
 * {"method":"update","state"{"reported":{"a":1}}}
 *
 * */
#define TC_IOT_MQTT_METHOD_UPDATE    "update"

/**< 删除服务端影子设备数据
 * {"method":"delete","state"{"desired":{"a":null}}}
 *
 * */
#define TC_IOT_MQTT_METHOD_DELETE    "delete"

/**< 更新设备状态控制指令
 * {"method":"control","state"{"reported":{"a":1}, "desired":{"a":2}}}
 * */
#define TC_IOT_MQTT_METHOD_CONTROL   "control"

/**< 上报固件信息（设备主动上报）
 * {"method":"update_firm_info","state":{"mac":"xxxxx", "uuid": 123455}}
 * */
#define TC_IOT_MQTT_METHOD_UPDATE_FIRM    "update_firm_info"
/**< 上报固件信息控制指令（服务端下发指令，要求客户端上报）
 * {"method":"report_firm_info"}
 * */
#define TC_IOT_MQTT_METHOD_REPORT_FIRM   "report_firm_info"
/* 响应类 */
/**< 读取请求响应*/
#define TC_IOT_MQTT_METHOD_REPLY     "reply"


/*--- end 影子设备请求响应包 method 字段取值----*/

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
int tc_iot_shadow_doc_pack_for_get_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    tc_iot_shadow_client *c);
int tc_iot_shadow_doc_pack_for_update_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    const char * reported, const char * desired,
                                    tc_iot_shadow_client *c) ;
int tc_iot_shadow_doc_pack_for_delete_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    const char * reported, const char * desired,
                                    tc_iot_shadow_client *c) ;

int tc_iot_shadow_doc_pack_start(char *buffer, int buffer_len,
                                 char * session_id, int session_id_len,
                                 const char * method,
                                 tc_iot_shadow_client *c);

int tc_iot_shadow_doc_pack_format(char *buffer, int buffer_len,
        const char * reported,
        const char * desired);
int tc_iot_shadow_doc_pack_end(char *buffer, int buffer_len, tc_iot_shadow_client *c);

int tc_iot_shadow_update_state(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        message_ack_handler callback, int timeout_ms, void * session_context,
         const char * state_name, int count, va_list p_args);

int tc_iot_shadow_update_firm_info(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        message_ack_handler callback, int timeout_ms, void * session_context,
         va_list p_args);
tc_iot_shadow_session * tc_iot_find_empty_session(tc_iot_shadow_client *c);
void tc_iot_release_session(tc_iot_shadow_session * p_session);

void tc_iot_device_on_message_received(tc_iot_message_data* md);
void _device_on_message_received(tc_iot_message_data* md);
int _tc_iot_sync_shadow_property(tc_iot_shadow_client * p_shadow_client,
        int property_total, tc_iot_shadow_property_def * properties, bool reported,
        const char * doc_start, jsmntok_t * json_token, int tok_count);

int tc_iot_shadow_doc_parse(tc_iot_shadow_client * p_shadow_client,
        const char * payload, jsmntok_t * json_token, int token_count, char * field_buf, int field_buf_len);

int tc_iot_shadow_event_notify(tc_iot_shadow_client * p_shadow_client, tc_iot_event_e event, void * data, void * context);

int tc_iot_shadow_cmp_local(tc_iot_shadow_client * c, int property_id, void * src, void * dest);
int tc_iot_shadow_cmp_local_with_reported(tc_iot_shadow_client * c, int property_id);
int tc_iot_shadow_cmp_local_with_desired(tc_iot_shadow_client * c, int property_id);
void * tc_iot_shadow_save_to_cached(tc_iot_shadow_client * c, int property_id, const void * p_data, void * p_cache);

int tc_iot_shadow_report_property(tc_iot_shadow_client * c, int property_id, char * buffer, int buffer_len);
int tc_iot_shadow_check_and_report(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        message_ack_handler callback, int timeout_ms, void * session_context, bool do_confirm);

tc_iot_shadow_property_def * tc_iot_shadow_get_property_def(tc_iot_shadow_client * p_shadow_client, int property_id);
const char * tc_iot_shadow_get_property_name(tc_iot_shadow_client * p_shadow_client, int property_id);
int tc_iot_shadow_get_property_type(tc_iot_shadow_client * p_shadow_client, int property_id);
int tc_iot_shadow_get_property_offset(tc_iot_shadow_client * p_shadow_client, int property_id);

int tc_iot_report_firm(tc_iot_shadow_client* p_shadow_client, ...);
int tc_iot_report_device_data(tc_iot_shadow_client* p_shadow_client);

int tc_iot_server_init(tc_iot_shadow_client* p_shadow_client, tc_iot_shadow_config * p_client_config);
int tc_iot_server_loop(tc_iot_shadow_client* p_shadow_client, int yield_timeout);
int tc_iot_server_destroy(tc_iot_shadow_client* p_shadow_client);

#endif /* end of include guard */

