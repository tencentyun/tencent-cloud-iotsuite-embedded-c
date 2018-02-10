#ifndef TC_IOT_CLIENT_H
#define TC_IOT_CLIENT_H

#include "tc_iot_inc.h"


int tc_iot_mqtt_client_construct(tc_iot_mqtt_client* p_mqtt_client,
                                 tc_iot_mqtt_client_config* p_client_config);

int tc_iot_mqtt_client_connect(tc_iot_mqtt_client* c, 
        tc_iot_mqtt_client_config* p_client_config);

void tc_iot_mqtt_client_destroy(tc_iot_mqtt_client* p_mqtt_client);

char tc_iot_mqtt_client_is_connected(tc_iot_mqtt_client* p_mqtt_client);
int tc_iot_mqtt_client_yield(tc_iot_mqtt_client* p_mqtt_client, int timeout_ms);
int tc_iot_mqtt_client_publish(tc_iot_mqtt_client* p_mqtt_client,
                               const char* topic, tc_iot_mqtt_message* msg);
int tc_iot_mqtt_client_subscribe(tc_iot_mqtt_client* p_mqtt_client,
                                 const char* topic_filter,
                                 tc_iot_mqtt_qos_e qos,
                                 message_handler msg_handler,
                                 void * context
                                 );
int tc_iot_mqtt_client_unsubscribe(tc_iot_mqtt_client* p_mqtt_client,
                                   const char* topic_filter);
int tc_iot_mqtt_client_disconnect(tc_iot_mqtt_client* p_mqtt_client);
#endif /* end of include guard */
