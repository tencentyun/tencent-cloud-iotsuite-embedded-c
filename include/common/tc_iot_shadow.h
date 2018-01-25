#ifndef TC_IOT_SHADOW_01171705_H
#define TC_IOT_SHADOW_01171705_H

typedef struct _tc_iot_shadow_config {
    tc_iot_mqtt_client_config mqtt_client_config;
    char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];
    char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];
    message_handler on_receive_msg;
} tc_iot_shadow_config;

#define SHADOW_CLIENT_SUB_TOPIC_TEMPLATE "shadow/get/%s/%s"
#define SHADOW_CLIENT_PUB_TOPIC_TEMPLATE "shadow/update/%s/%s"

typedef struct _tc_iot_shadow_client {
    tc_iot_shadow_config* p_shadow_config;
    tc_iot_mqtt_client mqtt_client;
} tc_iot_shadow_client;

int tc_iot_shadow_doc_pack_for_delete(char * buffer, int buffer_len, tc_iot_shadow_client *c, const char * reported, const char * desired);
int tc_iot_shadow_doc_pack_for_update(char * buffer, int buffer_len, tc_iot_shadow_client *c, const char * reported, const char * desired);
int tc_iot_shadow_doc_pack_for_get(char * buffer, int buffer_len, tc_iot_shadow_client *c);

#endif /* end of include guard */
