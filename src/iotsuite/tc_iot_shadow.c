#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

static void _tc_iot_shadow_on_message_received(tc_iot_message_data *md) {
    static jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];

    tc_iot_mqtt_message *message = md->message;
    tc_iot_shadow_client *c = md->context;
    char session_id[TC_IOT_SESSION_ID_LEN+1];
    tc_iot_shadow_session * session;
    int field_index = 0;
    int i;
    int ret;

    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        LOG_ERROR("BADFORMAT ->%.*s", (int)message->payloadlen, (char *)message->payload);
        return ;
    }

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, 
            "passthrough.sid", session_id, sizeof(session_id));
    if (field_index > 0 ) {
        for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
            session = &(c->sessions[i]);
            if (session->sid[0] != '\0' && strncmp(session->sid, session_id, strlen(session_id)) == 0) {
                if (session->handler) {
                    /* LOG_TRACE("session:%s response received", session->sid); */
                    session->handler(TC_IOT_ACK_SUCCESS, md, session->session_context);
                } else {
                    LOG_ERROR("session:%s handler not found", session->sid);
                }
                memset(session, 0, sizeof(*session));
                return ;
            }
        }
    } else {
        LOG_TRACE("field passthrough.sid not found, could be push from server.");
    }

    if (c && c->p_shadow_config && c->p_shadow_config->on_receive_msg) {
        c->p_shadow_config->on_receive_msg(md);
    } else {
        LOG_ERROR("UNHANDLED ->%.*s", (int)message->payloadlen, (char *)message->payload);
    }
}

int tc_iot_shadow_construct(tc_iot_shadow_client *c,
                            tc_iot_shadow_config *p_cfg) {
    int rc;
    tc_iot_mqtt_client_config *p_config;
    tc_iot_mqtt_client *p_mqtt_client;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_cfg, TC_IOT_NULL_POINTER);

    memset(c, 0, sizeof(tc_iot_shadow_client));

    c->p_shadow_config = p_cfg;
    p_config = &(p_cfg->mqtt_client_config);

    p_mqtt_client = &(c->mqtt_client);
    rc = tc_iot_mqtt_client_construct(p_mqtt_client, p_config);
    if (rc != TC_IOT_SUCCESS) {
        return rc;
    }

    rc = tc_iot_mqtt_client_subscribe(p_mqtt_client, p_cfg->sub_topic, TC_IOT_QOS1,
                                          _tc_iot_shadow_on_message_received, c);
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
                LOG_WARN("session:%s expired", session->sid);
                if (session->handler) {
                    session->handler(TC_IOT_ACK_TIMEOUT, NULL, session->session_context);
                } else {
                    LOG_ERROR("session:%s handler not found", session->sid);
                }
                memset(session, 0, sizeof(*session));
            } else {
                LOG_TRACE("session:%s not expired, left_ms=%d", session->sid,
                        tc_iot_hal_timer_left_ms(&(session->timer)));
            }
        }
    }
    return TC_IOT_SUCCESS;
}

int tc_iot_shadow_yield(tc_iot_shadow_client *c, int timeout_ms) {
	int ret;
	
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    ret = tc_iot_mqtt_client_yield(&(c->mqtt_client), timeout_ms);
    _tc_iot_check_expired_session(c);
	
	return ret;
}

tc_iot_shadow_session * tc_iot_find_empty_session(tc_iot_shadow_client *c) {
    int i;
    
    if (!c) {
        return NULL;
    }

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        if (strlen(c->sessions[i].sid) == 0) {
            return &(c->sessions[i]);
        }
    }
    return NULL;
}

int tc_iot_shadow_get(tc_iot_shadow_client *c, char * buffer, int buffer_len,  
         message_ack_handler callback, int timeout_ms, void * session_context) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    if (callback) {
        if (timeout_ms <= 0) {
            LOG_ERROR("callback handler given, but timeout_ms=%d", timeout_ms);
        }
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
        p_session->session_context = session_context;
    } else {
        rc = tc_iot_shadow_doc_pack_for_get_with_sid(buffer, buffer_len, NULL, 0, c);
    }

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

int tc_iot_shadow_update(tc_iot_shadow_client *c, char * buffer, int buffer_len, 
        const char * reported, const char * desired,
        message_ack_handler callback, int timeout_ms, void * session_context) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    if (callback) {
        if (timeout_ms <= 0) {
            LOG_ERROR("callback handler given, but timeout_ms=%d", timeout_ms);
        }
        p_session = tc_iot_find_empty_session(c);
        if (!p_session) {
            LOG_ERROR("no more empty session.");
            return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
        }
        rc = tc_iot_shadow_doc_pack_for_update_with_sid(buffer, buffer_len, &(p_session->sid[0]), 
                TC_IOT_SESSION_ID_LEN+1, reported, desired,
                c);
        tc_iot_hal_timer_init(&(p_session->timer));
        tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
        p_session->handler = callback;
        p_session->session_context = session_context;
    } else {
        rc = tc_iot_shadow_doc_pack_for_update_with_sid(buffer, buffer_len, NULL, 0, reported, desired, c);
    }

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

int tc_iot_shadow_doc_pack_for_update_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    const char * reported, const char * desired,
                                    tc_iot_shadow_client *c) {
    int ret;
    int buffer_used = 0;
    ret = tc_iot_shadow_doc_pack_start(buffer, buffer_len, session_id, session_id_len, TC_IOT_MQTT_METHOD_UPDATE, c);
    buffer_used += ret;
    ret = tc_iot_shadow_doc_pack_format(buffer+buffer_used, buffer_len-buffer_used, reported, desired);
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
            /* LOG_TRACE("sid_len=%d, sid=%.*s, ss=%s", sid_len, sid_len, session_id, session_id); */
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
    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len, "}");
    buffer_used += ret;
    return buffer_used;
}

int tc_iot_shadow_add_properties(char * buffer, int buffer_len, int property_total, tc_iot_shadow_property_def * properties, int property_count, va_list p_args)  {
    int ret = 0;
    int i = 0;
    int pos = 0;
    tc_iot_shadow_property_def * current = NULL;
    int prop_index = 0;
    void * p_prop;

    if(buffer == NULL) {
        return TC_IOT_NULL_POINTER;
    }

    ret = tc_iot_hal_snprintf(buffer + pos, buffer_len - pos, "{");
    if(ret <= 0) {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    pos += ret;

    for(i = 0; i < property_count; i++) {
        prop_index = va_arg (p_args, int);
        LOG_TRACE("No.%d param as prop index=%d", i, prop_index);
        current = &properties[prop_index];
        p_prop = va_arg (p_args, void *);
        if (i > 0) {
            ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos,",");
            if(ret <= 0) {
                return TC_IOT_BUFFER_OVERFLOW;
            }
            pos += ret;
        }

        if (p_prop == NULL) {
            ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos,"\"%s\":%s",
                    current->name, TC_IOT_JSON_NULL);
            if(ret <= 0) {
                return TC_IOT_BUFFER_OVERFLOW;
            }
            pos += ret;
            LOG_TRACE("buffer=%s", buffer);
            continue;
        }

        if (current->type == TC_IOT_SHADOW_TYPE_NUMBER) {
            ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos,"\"%s\":%d",
                    current->name, *(tc_iot_shadow_number *)p_prop);
            if(ret <= 0) {
                return TC_IOT_BUFFER_OVERFLOW;
            }
            pos += ret;
            /* LOG_TRACE("buffer=%s", buffer); */
        } else if (current->type == TC_IOT_SHADOW_TYPE_ENUM) {
            ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos,"\"%s\":%d",
                    current->name, *(tc_iot_shadow_enum *)p_prop);
            if(ret <= 0) {
                return TC_IOT_BUFFER_OVERFLOW;
            }
            pos += ret;
            //LOG_TRACE("buffer=%s", buffer);
        } else if (current->type == TC_IOT_SHADOW_TYPE_BOOL) {
            ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos,"\"%s\":%s",
                    current->name, *(tc_iot_shadow_bool *)p_prop ? TC_IOT_JSON_TRUE:TC_IOT_JSON_FALSE);
            if(ret <= 0) {
                return TC_IOT_BUFFER_OVERFLOW;
            }
            pos += ret;
            /* LOG_TRACE("buffer=%s", buffer); */
        } else {
            LOG_ERROR("%s type=%d unkown.", current->name, current->type);
            return TC_IOT_INVALID_PARAMETER;
        }
    }

    ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos,"}");
    pos += ret;
    return pos;
}

int tc_iot_shadow_update_state(tc_iot_shadow_client *c, char * buffer, int buffer_len, 
        message_ack_handler callback, int timeout_ms, void * session_context, 
         const char * state_name, int property_count,va_list p_args) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    int ret = 0;
    int i = 0;
    int pos = 0;
    
    tc_iot_shadow_property_def * current = NULL;
    int prop_index = 0;
    void * p_prop;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    p_session = tc_iot_find_empty_session(c);
    if (!p_session) {
        LOG_ERROR("no more empty session.");
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    }

    ret = tc_iot_shadow_doc_pack_start(buffer+pos, buffer_len-pos, &(p_session->sid[0]), TC_IOT_SESSION_ID_LEN+1, TC_IOT_MQTT_METHOD_UPDATE, c);
    if (ret <= 0) {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    pos += ret;

    ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, ",\"state\":{\"%s\":", state_name);
    if (ret <= 0) {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    pos += ret;

    ret = tc_iot_shadow_add_properties(buffer + pos, buffer_len - pos, 
            c->p_shadow_config->property_total, 
            c->p_shadow_config->properties, property_count, p_args);
    if (ret <= 0) {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    pos += ret;

    /* ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, ",\"desired\":"); */
    /* if (ret <= 0) { */
        /* return TC_IOT_BUFFER_OVERFLOW; */
    /* } */
    /* pos += ret; */

    /* va_start(p_args, property_count); */
    /* ret = tc_iot_shadow_add_desire_state(buffer + pos, buffer_len - pos, properties, property_count, p_args); */
    /* if (ret <= 0) { */
        /* return TC_IOT_BUFFER_OVERFLOW; */
    /* } */
    /* pos += ret; */
    /* va_end(p_args); */

    ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, "}");
    if (ret <= 0) {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    pos += ret;

    ret = tc_iot_shadow_doc_pack_end(buffer+pos, buffer_len-pos, c);
    if (ret <= 0) {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    pos += ret;

    tc_iot_hal_timer_init(&(p_session->timer));
    tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
    p_session->handler = callback;
    p_session->session_context = session_context;

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


int tc_iot_shadow_doc_pack_end(char *buffer, int buffer_len, tc_iot_shadow_client *c) {
    int ret;
    int buffer_used = 0;

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
