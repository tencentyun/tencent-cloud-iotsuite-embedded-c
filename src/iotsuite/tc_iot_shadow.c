#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

static void _on_message_receved(tc_iot_message_data *md) {
    tc_iot_mqtt_message *message = md->message;
    LOG_TRACE("->%.*s", (int)message->payloadlen, (char *)message->payload);
}

int tc_iot_shadow_construct(tc_iot_shadow_client *c,
                            tc_iot_shadow_config *p_cfg) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_cfg, TC_IOT_NULL_POINTER);

    c->p_shadow_config = p_cfg;
    tc_iot_mqtt_client_config *p_config = &(p_cfg->mqtt_client_config);

    tc_iot_mqtt_client *p_mqtt_client = &(c->mqtt_client);
    tc_iot_mqtt_client_construct(p_mqtt_client, p_config);

    char *product_id = p_config->device_info.product_id;
    char *device_name = p_config->device_info.device_name;

    message_handler msg_handler;
    if (p_cfg->on_receive_msg) {
        msg_handler = p_cfg->on_receive_msg;
    } else {
        msg_handler = _on_message_receved;
    }

    int rc = tc_iot_mqtt_client_subscribe(p_mqtt_client, p_cfg->sub_topic, QOS1,
                                          msg_handler);
    if (TC_IOT_SUCCESS == rc) {
        LOG_DEBUG("Subscribing to %s success.", p_cfg->sub_topic);
    } else {
        LOG_ERROR("!!!Subscribing to %s failed, ret code=%d.", p_cfg->sub_topic,
                  rc);
    }
    return TC_IOT_SUCCESS;
}

void tc_iot_shadow_destroy(tc_iot_shadow_client *c) {
    if (c) {
        tc_iot_mqtt_client_disconnect(&(c->mqtt_client));
    }
}

char tc_iot_shadow_isconnected(tc_iot_shadow_client *c) {
    IF_NULL_RETURN(c, 0);
    return tc_iot_mqtt_client_is_connected(&(c->mqtt_client));
}

int tc_iot_shadow_yield(tc_iot_shadow_client *c, int timeout_ms) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    tc_iot_mqtt_client_yield(&(c->mqtt_client), timeout_ms);
}

int tc_iot_shadow_get(tc_iot_shadow_client *c) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    char *action_get = "{\"method\":\"get\"}";
    tc_iot_mqtt_message pubmsg;
    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = action_get;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    LOG_DEBUG("requesting with: %.*s", (int)pubmsg.payloadlen,
              (char *)pubmsg.payload);
    char *pub_topic = c->p_shadow_config->pub_topic;
    int rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("!!!tc_iot_mqtt_client_publish failed, return=-0x%X", rc);
    }
    return rc;
}

int tc_iot_shadow_update(tc_iot_shadow_client *c, char *p_json) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_json, TC_IOT_NULL_POINTER);

    tc_iot_mqtt_message pubmsg;
    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = p_json;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    LOG_DEBUG("requesting with: %.*s", (int)pubmsg.payloadlen,
              (char *)pubmsg.payload);
    char *pub_topic = c->p_shadow_config->pub_topic;
    int rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("!!!tc_iot_mqtt_client_publish failed, return=-0x%X", rc);
    }
    return rc;
}

int tc_iot_shadow_delete(tc_iot_shadow_client *c, char *p_json) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    tc_iot_mqtt_message pubmsg;
    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = p_json;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    LOG_DEBUG("requesting with: %.*s", (int)pubmsg.payloadlen,
              (char *)pubmsg.payload);
    char *pub_topic = c->p_shadow_config->pub_topic;
    int rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("!!!tc_iot_mqtt_client_publish failed, return=-0x%X", rc);
    }
    return rc;
}

#ifdef __cplusplus
}
#endif
