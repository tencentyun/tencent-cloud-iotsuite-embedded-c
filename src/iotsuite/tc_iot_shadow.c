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
    
    char *product_id;
    char *device_name;
    int rc;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_cfg, TC_IOT_NULL_POINTER);

    c->p_shadow_config = p_cfg;
    tc_iot_mqtt_client_config *p_config = &(p_cfg->mqtt_client_config);

    tc_iot_mqtt_client *p_mqtt_client = &(c->mqtt_client);
    rc = tc_iot_mqtt_client_construct(p_mqtt_client, p_config);
    if (rc != TC_IOT_SUCCESS) {
        return rc;
    }

    product_id = p_config->device_info.product_id;
    device_name = p_config->device_info.device_name;

    message_handler msg_handler;
    if (p_cfg->on_receive_msg) {
        msg_handler = p_cfg->on_receive_msg;
    } else {
        msg_handler = _on_message_receved;
    }

    rc = tc_iot_mqtt_client_subscribe(p_mqtt_client, p_cfg->sub_topic, TC_IOT_QOS1,
                                          msg_handler);
    if (TC_IOT_SUCCESS == rc) {
        LOG_TRACE("subscribing to %s success.", p_cfg->sub_topic);
    } else {
        LOG_ERROR("subscribing to %s failed, ret code=%d.", p_cfg->sub_topic,
                  rc);
    }
    return rc;
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
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    LOG_TRACE("requesting with: %.*s", (int)pubmsg.payloadlen,
              (char *)pubmsg.payload);
    char *pub_topic = c->p_shadow_config->pub_topic;
    int rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
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
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    LOG_TRACE("requesting with: %.*s", (int)pubmsg.payloadlen,
              (char *)pubmsg.payload);
    char *pub_topic = c->p_shadow_config->pub_topic;
    int rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }
    return rc;
}

int tc_iot_shadow_delete(tc_iot_shadow_client *c, char *p_json) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    tc_iot_mqtt_message pubmsg;
    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = p_json;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    LOG_TRACE("requesting with: %.*s", (int)pubmsg.payloadlen,
              (char *)pubmsg.payload);
    char *pub_topic = c->p_shadow_config->pub_topic;
    int rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }
    return rc;
}

int tc_iot_shadow_doc_pack_for_delete(char *buffer, int buffer_len,
                                      tc_iot_shadow_client *c,
                                      const char *reported,
                                      const char *desired) {
    int ret;
    int buffer_used = 0;

    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                              "{\"method\":\"%s\"", TC_IOT_MQTT_METHOD_DELETE);
    buffer_used += ret;
    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len, ",\"state\":{");
    buffer_used += ret;
    if (reported && desired) {
        ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                                  "\"reported\":%s,\"desired\":%s", reported,
                                  desired);
        buffer_used += ret;
    } else if (reported) {
        ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                                  "\"reported\":%s", reported);
        buffer_used += ret;
    } else if (desired) {
        ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                                  "\"desired\":%s", desired);
        buffer_used += ret;
    } else {
        LOG_ERROR("both reported and desired not given.");
        return TC_IOT_INVALID_PARAMETER;
    }

    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len, "}}");
    buffer_used += ret;

    if (buffer_used < buffer_len) {
        buffer[buffer_used] = '\0';
    }

    return buffer_used;
}

int tc_iot_shadow_doc_pack_for_update(char *buffer, int buffer_len,
                                      tc_iot_shadow_client *c,
                                      const char *reported,
                                      const char *desired) {
    int ret;
    int buffer_used = 0;

    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                              "{\"method\":\"%s\"", TC_IOT_MQTT_METHOD_UPDATE);
    buffer_used += ret;
    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len, ",\"state\":{");
    buffer_used += ret;
    if (reported && desired) {
        ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                                  "\"reported\":%s,\"desired\":%s", reported,
                                  desired);
        buffer_used += ret;
    } else if (reported) {
        ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                                  "\"reported\":%s", reported);
        buffer_used += ret;
    } else if (desired) {
        ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                                  "\"desired\":%s", desired);
        buffer_used += ret;
    } else {
        LOG_ERROR("both reported and desired not given.");
        return TC_IOT_INVALID_PARAMETER;
    }

    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len, "}}");
    buffer_used += ret;
    if (buffer_used < buffer_len) {
        buffer[buffer_used] = '\0';
    }

    return buffer_used;
}

int tc_iot_shadow_doc_pack_for_get(char *buffer, int buffer_len,
                                   tc_iot_shadow_client *c) {
    int ret;
    int buffer_used = 0;

    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                              "{\"method\":\"%s\"}", TC_IOT_MQTT_METHOD_GET);
    buffer_used += ret;
    if (buffer_used < buffer_len) {
        buffer[buffer_used] = '\0';
    }

    return buffer_used;
}

#ifdef __cplusplus
}
#endif
