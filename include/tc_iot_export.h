#ifndef IOT_EXPORT_H
#define IOT_EXPORT_H

#include "tc_iot_inc.h"

void tc_iot_set_log_level(int log_level);
int tc_iot_get_log_level();
bool tc_iot_log_level_enabled(int log_level);

int tc_iot_mqtt_client_construct(tc_iot_mqtt_client* p_mqtt_client,
                                 tc_iot_mqtt_client_config* p_client_config);
void tc_iot_mqtt_client_destroy(tc_iot_mqtt_client* p_mqtt_client);
char tc_iot_mqtt_client_is_connected(tc_iot_mqtt_client* p_mqtt_client);
int tc_iot_mqtt_client_yield(tc_iot_mqtt_client* p_mqtt_client, int timeout_ms);
int tc_iot_mqtt_client_publish(tc_iot_mqtt_client* p_mqtt_client,
                               const char* topic, tc_iot_mqtt_message* msg);
int tc_iot_mqtt_client_subscribe(tc_iot_mqtt_client* p_mqtt_client,
                                 const char* topic_filter,
                                 tc_iot_mqtt_qos_e qos,
                                 message_handler msg_handler);
int tc_iot_mqtt_client_unsubscribe(tc_iot_mqtt_client* p_mqtt_client,
                                   const char* topic_filter);
int tc_iot_mqtt_client_disconnect(tc_iot_mqtt_client* p_mqtt_client);

int tc_iot_shadow_construct(tc_iot_shadow_client *,
                            tc_iot_shadow_config *p_config);
void tc_iot_shadow_destroy(tc_iot_shadow_client *p_shadow_client);

char tc_iot_shadow_isconnected(tc_iot_shadow_client *p_shadow_client);
int tc_iot_shadow_yield(tc_iot_shadow_client *p_shadow_client, int timeout_ms);
int tc_iot_shadow_get(tc_iot_shadow_client *p_shadow_client);
int tc_iot_shadow_update(tc_iot_shadow_client *p_shadow_client, char *pJsonDoc);
int tc_iot_shadow_delete(tc_iot_shadow_client *p_shadow_client, char *pJsonDoc);

#endif /* end of include guard */
