#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

static void _on_message_receved(tc_iot_message_data *md) {
    tc_iot_mqtt_message *message = md->message;
    tc_iot_shadow_client *c = md->context;
    char session_id[TC_IOT_SESSION_ID_LEN+1];

    if (c && c->p_shadow_config && c->p_shadow_config->on_receive_msg) {
        c->p_shadow_config->on_receive_msg(md);
    } else {
        LOG_ERROR("UNHANDLED ->%.*s", (int)message->payloadlen, (char *)message->payload);
    }
}

int tc_iot_shadow_construct(tc_iot_shadow_client *c,
                            tc_iot_shadow_config *p_cfg) {
    
    char *product_id;
    char *device_name;
    int rc;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_cfg, TC_IOT_NULL_POINTER);

    memset(c, 0, sizeof(tc_iot_shadow_client));

    c->p_shadow_config = p_cfg;
    tc_iot_mqtt_client_config *p_config = &(p_cfg->mqtt_client_config);

    product_id = p_config->device_info.product_id;
    device_name = p_config->device_info.device_name;


    tc_iot_mqtt_client *p_mqtt_client = &(c->mqtt_client);
    rc = tc_iot_mqtt_client_construct(p_mqtt_client, p_config);
    if (rc != TC_IOT_SUCCESS) {
        return rc;
    }

    rc = tc_iot_mqtt_client_subscribe(p_mqtt_client, p_cfg->sub_topic, TC_IOT_QOS1,
                                          _on_message_receved, c);
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

static int _tc_iot_check_expired_session(tc_iot_shadow_client *c) {
    int i;
    tc_iot_shadow_session * session;
    
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        session = &(c->sessions[i]);
        if (session->sid[0] != 0) {
            if (tc_iot_hal_timer_is_expired(&(session->timer))) {
                LOG_TRACE("session:%s expired", session->sid);
                if (session->handler) {
                    session->handler(TC_IOT_ACK_TIMEOUT, NULL, session->context);
                } else {
                    LOG_ERROR("session:%s handler not found", &(session->sid[0]));
                }
                memset(session, 0, sizeof(*session));
            } else {
                LOG_TRACE("session:%s not expired, left_ms=%d", &(session->sid[0]),
                        tc_iot_hal_timer_left_ms(&(session->timer)));
            }
        }
    }
    return TC_IOT_SUCCESS;
}

int tc_iot_shadow_yield(tc_iot_shadow_client *c, int timeout_ms) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    tc_iot_mqtt_client_yield(&(c->mqtt_client), timeout_ms);
    _tc_iot_check_expired_session(c);
}

tc_iot_shadow_session * tc_iot_find_empty_session(tc_iot_shadow_client *c) {
    int i;
    
    IF_NULL_RETURN(c, NULL);

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        if (strlen(c->sessions[i].sid) == 0) {
            return &(c->sessions[i]);
        }
    }
    return NULL;
}

int tc_iot_shadow_get(tc_iot_shadow_client *c, char * buffer, int buffer_len,  
         message_ack_handler callback, int timeout_ms, void * context) {
    char *pub_topic ;
    int rc ;
    char session_id[TC_IOT_SESSION_ID_LEN+1];
    tc_iot_shadow_session * p_session;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    if (callback) {
        p_session = tc_iot_find_empty_session(c);
        if (!p_session) {
            LOG_ERROR("no more empty session.");
            return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
        }
        rc = tc_iot_shadow_doc_pack_for_get_with_sid(buffer, buffer_len, &(p_session->sid[0]), 
                TC_IOT_SESSION_ID_LEN+1, c);
        tc_iot_hal_timer_init(&(p_session->timer));
        tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
        p_session->handler = callback;
        p_session->context = context;
    } else {
        rc = tc_iot_shadow_doc_pack_for_get_with_sid(buffer, buffer_len, NULL, 0, c);
    }

    tc_iot_mqtt_message pubmsg;
    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    LOG_TRACE("requesting with: %.*s", (int)pubmsg.payloadlen,
              (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }
    return rc;
}

int tc_iot_shadow_update(tc_iot_shadow_client *c, char *p_json) {
    char *pub_topic ;
    int rc ;

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
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }
    return rc;
}

int tc_iot_shadow_delete(tc_iot_shadow_client *c, char *p_json) {
    char *pub_topic;
    int rc;

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
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }
    return rc;
}

static unsigned int  _get_unique_session_id(tc_iot_mqtt_client* c) {
    static unsigned short usid = 0;
    unsigned int ret =  ((c->client_init_time << 16) & 0xFFFF0000) | (0xFFFF & (usid++));
    return ret;
}


static int  _generate_session_id(char * session_id, int session_id_len, tc_iot_mqtt_client* c) {
    unsigned int sid = _get_unique_session_id(c);
    int ret ;
    ret = tc_iot_hal_snprintf(session_id, session_id_len, "%x", sid);
    if (ret > 0 && ret < session_id_len) {
        session_id[ret] = '\0';
    }
    return ret;
}

int tc_iot_shadow_doc_pack_for_delete(char *buffer, int buffer_len,
                                      tc_iot_shadow_client *c,
                                      const char *reported,
                                      const char *desired) {
    int ret;
    int buffer_used = 0;

    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                              "{\"method\":\"%s\",\"passthrough\":{\"m\":\"d\",\"sid\":\"%x\"}", 
                              TC_IOT_MQTT_METHOD_DELETE, 
                              _get_unique_session_id(&(c->mqtt_client)));
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
                              "{\"method\":\"%s\",\"passthrough\":{\"m\":\"u\",\"sid\":\"%x\"}", 
                              TC_IOT_MQTT_METHOD_UPDATE,
                              _get_unique_session_id(&(c->mqtt_client))
                              );
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
    return tc_iot_shadow_doc_pack_for_get_with_sid(buffer, buffer_len, NULL, 0, c);
}

int tc_iot_shadow_doc_pack_for_get_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    tc_iot_shadow_client *c) {
    int ret;
    int buffer_used = 0;
    ret = tc_iot_shadow_doc_pack_start(buffer, buffer_len, session_id, session_id_len, TC_IOT_MQTT_METHOD_GET, c);
    buffer_used += ret;
    ret = tc_iot_shadow_doc_pack_end(buffer+buffer_used, buffer_len-buffer_used, c);
    buffer_used += ret;
    return buffer_used;
}

int tc_iot_shadow_doc_pack_start(char *buffer, int buffer_len,
                                 char * session_id, int session_id_len,
                                 const char * method,
                                 tc_iot_shadow_client *c) {
    int ret;
    int buffer_used = 0;
    int sid_len = 0;

    if (session_id && (session_id_len >= TC_IOT_SESSION_ID_LEN)) {
        sid_len = _generate_session_id(session_id, session_id_len, &(c->mqtt_client));
        if (sid_len <= 0) {
            LOG_ERROR("generate session id failed: sid_len=%d", sid_len);
            memset(session_id, '0', TC_IOT_SESSION_ID_LEN);
            sid_len = TC_IOT_SESSION_ID_LEN;
        } else {
            LOG_TRACE("sid_len=%d, sid=%.*s, ss=%s", sid_len, sid_len, session_id, session_id);
        }

        ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len,
                "{\"method\":\"%s\",\"passthrough\":{\"sid\":\"%.*s\"}", 
                method,
                sid_len,
                session_id
                );
    } else {
        ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len, "{\"method\":\"%s\"", method);
    }

    buffer_used += ret;
    return buffer_used;
}

int tc_iot_shadow_doc_pack_format(char *buffer, int buffer_len, const char * reported, const char * desired) {
    int ret;
    int buffer_used = 0;

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
    return buffer_used;
}

int tc_iot_shadow_doc_pack_end(char *buffer, int buffer_len, tc_iot_shadow_client *c) {
    int ret;
    int buffer_used = 0;
    int sid_len = 0;

    ret = tc_iot_hal_snprintf(buffer, buffer_len, "}");

    buffer_used += ret;
    if (buffer_used < buffer_len) {
        buffer[buffer_used] = '\0';
    }

    return buffer_used;
}

#ifdef __cplusplus
}
#endif
