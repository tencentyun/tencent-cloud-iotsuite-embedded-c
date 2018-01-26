#ifndef TC_IOT_MQTT_01091046_H
#define TC_IOT_MQTT_01091046_H

#include "tc_iot_inc.h"

#define MAX_PACKET_ID 65535
#define MAX_MESSAGE_HANDLERS 5

typedef enum _tc_iot_mqtt_qos_e {
    QOS0 = 0,
    QOS1 = 1,
    QOS2 = 2,
    SUBFAIL = 0x80,
} tc_iot_mqtt_qos_e;

typedef struct _tc_iot_mqtt_message {
    tc_iot_mqtt_qos_e qos;
    unsigned char retained;
    unsigned char dup;
    unsigned short id;
    void* payload;
    size_t payloadlen;
} tc_iot_mqtt_message;

typedef struct _tc_iot_message_data {
    tc_iot_mqtt_message* message;
    MQTTString* topicName;
} tc_iot_message_data;

typedef struct _tc_iot_mqtt_connack_data {
    unsigned char rc;
    unsigned char sessionPresent;
} tc_iot_mqtt_connack_data;

typedef struct _tc_iot_mqtt_suback_data {
    tc_iot_mqtt_qos_e grantedQoS;
} tc_iot_mqtt_suback_data;

typedef void (*message_handler)(tc_iot_message_data*);

typedef enum _tc_iot_mqtt_client_state_e {
    CLIENT_INVALID,
    CLIENT_INTIALIAZED,
    CLIENT_NETWORK_READY,
    CLIENT_CONNECTED,
} tc_iot_mqtt_client_state_e;

typedef struct _tc_iot_mqtt_client tc_iot_mqtt_client;
typedef void (*disconnectHandler)(tc_iot_mqtt_client*, void*);
typedef void (*defaultMessageHandler)(tc_iot_message_data*);

typedef struct _tc_iot_mqtt_client {
    unsigned int command_timeout_ms;
    size_t buf_size;
    size_t readbuf_size;
    unsigned char buf[TC_IOT_CLIENT_SEND_BUF_SIZE];
    unsigned char readbuf[TC_IOT_CLIENT_READ_BUF_SIZE];
    unsigned int keep_alive_interval;
    char auto_reconnect;
    unsigned int reconnect_timeout_ms;
    int clean_session;
    unsigned int next_packetid;
    char ping_outstanding;
    tc_iot_mqtt_client_state_e state;
    MQTTPacket_connectData connect_options;

    struct MessageHandlers {
        const char* topicFilter;
        void (*fp)(tc_iot_message_data*);
    } message_handlers[MAX_MESSAGE_HANDLERS];

    defaultMessageHandler default_msg_handler;
    disconnectHandler disconnect_handler;

    tc_iot_network_t ipstack;
    tc_iot_timer last_sent;
    tc_iot_timer last_received;
    tc_iot_timer reconnect_timer;
} tc_iot_mqtt_client;

typedef struct _tc_iot_device_info {
    char secret[TC_IOT_MAX_SECRET_LEN];
    char product_id[TC_IOT_MAX_PRODUCT_ID_LEN];
    char device_name[TC_IOT_MAX_DEVICE_NAME_LEN];
    char client_id[TC_IOT_MAX_CLIENT_ID_LEN];

    char username[TC_IOT_MAX_USER_NAME_LEN];
    char password[TC_IOT_MAX_PASSWORD_LEN];
    long token_expire_time; /* username&password would expire */
} tc_iot_device_info;

typedef struct _tc_iot_mqtt_client_config {
    tc_iot_device_info device_info;
    char* host;
    uint16_t port;
    int command_timeout_ms;
    int keep_alive_interval; /* second */
    char clean_session;
    char use_tls;
    char auto_reconnect;
    const char* p_root_ca;
    const char* p_client_crt;
    const char* p_client_key;

    disconnectHandler disconnect_handler;
    defaultMessageHandler default_msg_handler;
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
                                    message_handler message_handler);
int tc_iot_mqtt_subscribe(tc_iot_mqtt_client* client, const char* topicFilter,
                          tc_iot_mqtt_qos_e, message_handler);
int tc_iot_mqtt_subscribe_with_results(tc_iot_mqtt_client* client,
                                       const char* topicFilter,
                                       tc_iot_mqtt_qos_e, message_handler,
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

#endif /* end of include guard */
