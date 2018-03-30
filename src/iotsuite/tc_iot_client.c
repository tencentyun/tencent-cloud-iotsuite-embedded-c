#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_mqtt_client_construct(tc_iot_mqtt_client* c,
                                 tc_iot_mqtt_client_config* p_client_config) {
    int rc;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_client_config, TC_IOT_NULL_POINTER);


    rc = tc_iot_mqtt_init(c, p_client_config);
    if (rc != TC_IOT_SUCCESS) {
        return rc;
    }

    rc = tc_iot_mqtt_client_connect(c, p_client_config);
    return rc;
}

int tc_iot_mqtt_client_connect(tc_iot_mqtt_client* c,
                                 tc_iot_mqtt_client_config* p_client_config) {
    int rc;
	MQTTPacket_connectData* data;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_client_config, TC_IOT_NULL_POINTER);
    
    if (tc_iot_mqtt_client_is_connected(c)) {
        return TC_IOT_SUCCESS;
    }

    data = &(c->connect_options);
    tc_iot_init_mqtt_conn_data(data);;
    data->willFlag = p_client_config->willFlag;
    data->will = p_client_config->will;
    data->MQTTVersion = 4; /*4 means MQTT 3.1.1 */
    data->clientID.cstring = p_client_config->device_info.client_id;
    data->username.cstring = p_client_config->device_info.username;
    data->password.cstring = p_client_config->device_info.password;
    data->keepAliveInterval = p_client_config->keep_alive_interval;
    data->cleansession = p_client_config->clean_session;

    rc = tc_iot_mqtt_connect(c, data);
    if (TC_IOT_SUCCESS == rc) {
        TC_IOT_LOG_TRACE("mqtt client connect %s:%d success", p_client_config->host,
                  p_client_config->port);
    } else {
        TC_IOT_LOG_ERROR("mqtt cllient connect %s:%d failed retcode %d",
                  p_client_config->host, p_client_config->port, rc);
    }
    return rc;
}

void tc_iot_mqtt_client_destroy(tc_iot_mqtt_client* c) {
    tc_iot_mqtt_destroy(c);
}

char tc_iot_mqtt_client_is_connected(tc_iot_mqtt_client* c) {
    if (!c) {
		    return 0;
		}
    return tc_iot_mqtt_is_connected(c);
}

int tc_iot_mqtt_client_yield(tc_iot_mqtt_client* c, int timeout_ms) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    return tc_iot_mqtt_yield(c, timeout_ms);
}

int tc_iot_mqtt_client_publish(tc_iot_mqtt_client* c, const char* topic,
                               tc_iot_mqtt_message* msg) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topic, TC_IOT_NULL_POINTER);
    return tc_iot_mqtt_publish(c, topic, msg);
}

int tc_iot_mqtt_client_subscribe(tc_iot_mqtt_client* c,
                                 const char* topic_filter,
                                 tc_iot_mqtt_qos_e qos,
                                 message_handler msg_handler, void * context) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topic_filter, TC_IOT_NULL_POINTER);
    return tc_iot_mqtt_subscribe(c, topic_filter, qos, msg_handler, context);
}

int tc_iot_mqtt_client_unsubscribe(tc_iot_mqtt_client* c,
                                   const char* topic_filter) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topic_filter, TC_IOT_NULL_POINTER);
    return tc_iot_mqtt_unsubscribe(c, topic_filter);
}

int tc_iot_mqtt_client_disconnect(tc_iot_mqtt_client* c) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    return tc_iot_mqtt_disconnect(c);
}

#ifdef __cplusplus
}
#endif
