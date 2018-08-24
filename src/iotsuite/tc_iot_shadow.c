#include "tc_iot_inc.h"

static void _tc_iot_shadow_on_message_received(tc_iot_message_data *md) {
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];

    tc_iot_mqtt_message *message = md->message;
    tc_iot_shadow_client *c = md->context;
    char session_id[TC_IOT_SESSION_ID_LEN+1];
    tc_iot_shadow_session * session;
    int field_index = 0;
    int i;
    int ret;

    if (md->error_code != TC_IOT_SUCCESS) {
        if (c && c->p_shadow_config && c->p_shadow_config->on_receive_msg) {
            c->p_shadow_config->on_receive_msg(md);
        }
        return;
    }

    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        if (TC_IOT_JSON_PARSE_TOKEN_NO_MEM == ret) {
            TC_IOT_LOG_ERROR("change TC_IOT_MAX_JSON_TOKEN_COUNT larger, mem not enough ->%s", (char *)message->payload);
        } else {
            TC_IOT_LOG_ERROR("BADFORMAT ->%s", (char *)message->payload);
        }
        return ;
    }

    tc_iot_mem_usage_log("json_token[TC_IOT_MAX_JSON_TOKEN_COUNT]", sizeof(json_token), sizeof(json_token[0])*ret);

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret,
            "passthrough.sid", session_id, sizeof(session_id));
    if (field_index > 0 ) {
        for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
            session = &(c->sessions[i]);
            if (session->sid[0] != '\0' && strncmp(session->sid, session_id, strlen(session_id)) == 0) {
                if (session->handler) {
                    /* TC_IOT_LOG_TRACE("session:%s response received", session->sid); */
                    session->handler(TC_IOT_ACK_SUCCESS, md, session->session_context);
                } else {
                    TC_IOT_LOG_ERROR("session:%s handler not found", session->sid);
                }
                tc_iot_release_session(session);
                return ;
            }
        }
    } else {
        TC_IOT_LOG_TRACE("field passthrough.sid not found, could be push from server.");
    }

    if (c && c->p_shadow_config && c->p_shadow_config->on_receive_msg) {
        c->p_shadow_config->on_receive_msg(md);
    } else {
        TC_IOT_LOG_ERROR("UNHANDLED ->%s", (char *)message->payload);
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
        TC_IOT_LOG_TRACE("subscribing to %s success.", p_cfg->sub_topic);
    } else {
        TC_IOT_LOG_ERROR("subscribing to %s failed, ret code=%d.", p_cfg->sub_topic,
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
                TC_IOT_LOG_WARN("session:%s expired", session->sid);
                if (session->handler) {
                    session->handler(TC_IOT_ACK_TIMEOUT, NULL, session->session_context);
                } else {
                    TC_IOT_LOG_ERROR("session:%s handler not found", session->sid);
                }
                tc_iot_release_session(session);
            } else {
                /* TC_IOT_LOG_TRACE("session:%s not expired, left_ms=%d", session->sid, */
                        /* tc_iot_hal_timer_left_ms(&(session->timer))); */
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

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        TC_IOT_LOG_TRACE("occupied sid[%d]:%s", i, c->sessions[i].sid);
    }
    return NULL;
}

int tc_iot_shadow_pending_session_count(tc_iot_shadow_client *c) {
    int i;
    int total = 0;

    if (!c) {
        return 0;
    }

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        if (strlen(c->sessions[i].sid) != 0) {
            total++;
        }
    }
    return total;
}

void tc_iot_release_session(tc_iot_shadow_session * p_session) {
    if (!p_session) {
        return ;
    }

    TC_IOT_LOG_TRACE("sid released:%s", p_session->sid)
    memset(p_session, 0, sizeof(tc_iot_shadow_session));
    return ;
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
            TC_IOT_LOG_ERROR("callback handler given, but timeout_ms=%d", timeout_ms);
        }
        p_session = tc_iot_find_empty_session(c);
        if (!p_session) {
            TC_IOT_LOG_ERROR("no more empty session.");
            return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
        }
        rc = tc_iot_shadow_doc_pack_for_get_with_sid(buffer, buffer_len, &(p_session->sid[0]),
                TC_IOT_SESSION_ID_LEN+1,  false, true,
                c);
        if (rc < 0) {
            TC_IOT_LOG_ERROR("tc_iot_shadow_doc_pack_for_get_with_sid failed, return=%d", rc);
            tc_iot_release_session(p_session);
            return rc;
        }
        tc_iot_hal_timer_init(&(p_session->timer));
        tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
        p_session->handler = callback;
        p_session->session_context = session_context;
    } else {
        rc = tc_iot_shadow_doc_pack_for_get_with_sid(buffer, buffer_len, NULL, 0, false, true, c);
        if (rc < 0) {
            TC_IOT_LOG_ERROR("tc_iot_shadow_doc_pack_for_get_with_sid failed, return=%d", rc);
            return rc;
        }
    }


    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    /*TC_IOT_LOG_TRACE("requesting with: %s", (char *)pubmsg.payload);*/
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
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
            TC_IOT_LOG_ERROR("callback handler given, but timeout_ms=%d", timeout_ms);
        }
        p_session = tc_iot_find_empty_session(c);
        if (!p_session) {
            TC_IOT_LOG_ERROR("no more empty session.");
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
    TC_IOT_LOG_TRACE("requesting with: %s", (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
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


int tc_iot_shadow_doc_pack_for_get_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    bool metadata, bool reported,
                                    tc_iot_shadow_client *c) {
    int ret;
    int sid_len;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_GET);

    if (session_id && (session_id_len >= TC_IOT_SESSION_ID_LEN)) {
        sid_len = _generate_session_id(session_id, session_id_len, &(c->mqtt_client));
        if (sid_len <= 0) {
            TC_IOT_LOG_ERROR("generate session id failed: sid_len=%d", sid_len);
            memset(session_id, '0', TC_IOT_SESSION_ID_LEN);
            sid_len = TC_IOT_SESSION_ID_LEN;
        } else {
            tc_iot_json_writer_object_begin(w ,"passthrough");
            
            tc_iot_json_writer_string(w ,"sid", session_id);
            tc_iot_json_writer_object_end(w);
        }
    }

    tc_iot_json_writer_bool(w ,"metadata", metadata);
    tc_iot_json_writer_bool(w ,"reported", reported);
    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    }

    return ret;
}

int tc_iot_shadow_doc_pack_for_update_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    const char * reported, const char * desired,
                                    tc_iot_shadow_client *c) {
    int ret;
    int sid_len;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_UPDATE);

    if (session_id && (session_id_len >= TC_IOT_SESSION_ID_LEN)) {
        sid_len = _generate_session_id(session_id, session_id_len, &(c->mqtt_client));
        if (sid_len <= 0) {
            TC_IOT_LOG_ERROR("generate session id failed: sid_len=%d", sid_len);
            memset(session_id, '0', TC_IOT_SESSION_ID_LEN);
            sid_len = TC_IOT_SESSION_ID_LEN;
        } else {
            tc_iot_json_writer_object_begin(w ,"passthrough");
            
            tc_iot_json_writer_string(w ,"sid", session_id);
            tc_iot_json_writer_object_end(w);
        }
    }

    tc_iot_json_writer_object_begin(w ,"state");
    if (reported) {
        tc_iot_json_writer_raw_data(w ,"reported", reported);
    }

    if (desired) {
        tc_iot_json_writer_raw_data(w ,"desired", desired);
    }
    tc_iot_json_writer_object_end(w);

    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    }

    return ret;
}

int tc_iot_shadow_delete(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        const char * reported, const char * desired,
        message_ack_handler callback, int timeout_ms, void * session_context) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    if (callback) {
        if (timeout_ms <= 0) {
            TC_IOT_LOG_ERROR("callback handler given, but timeout_ms=%d", timeout_ms);
        }
        p_session = tc_iot_find_empty_session(c);
        if (!p_session) {
            TC_IOT_LOG_ERROR("no more empty session.");
            return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
        }
        rc = tc_iot_shadow_doc_pack_for_delete_with_sid(buffer, buffer_len, &(p_session->sid[0]),
                TC_IOT_SESSION_ID_LEN+1, reported, desired,
                c);
        tc_iot_hal_timer_init(&(p_session->timer));
        tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
        p_session->handler = callback;
        p_session->session_context = session_context;
    } else {
        rc = tc_iot_shadow_doc_pack_for_delete_with_sid(buffer, buffer_len, NULL, 0, reported, desired, c);
    }

    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    TC_IOT_LOG_TRACE("requesting with: %s", (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }
    return rc;
}

int tc_iot_shadow_doc_pack_for_delete_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    const char * reported, const char * desired,
                                    tc_iot_shadow_client *c) {
    int ret;
    int sid_len;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_DELETE);

    if (session_id && (session_id_len >= TC_IOT_SESSION_ID_LEN)) {
        sid_len = _generate_session_id(session_id, session_id_len, &(c->mqtt_client));
        if (sid_len <= 0) {
            TC_IOT_LOG_ERROR("generate session id failed: sid_len=%d", sid_len);
            memset(session_id, '0', TC_IOT_SESSION_ID_LEN);
            sid_len = TC_IOT_SESSION_ID_LEN;
        } else {
            tc_iot_json_writer_object_begin(w ,"passthrough");
            
            tc_iot_json_writer_string(w ,"sid", session_id);
            tc_iot_json_writer_object_end(w);
        }
    }

    tc_iot_json_writer_object_begin(w ,"state");
    if (reported) {
        tc_iot_json_writer_raw_data(w ,"reported", reported);
    }

    if (desired) {
        tc_iot_json_writer_raw_data(w ,"desired", desired);
    }
    tc_iot_json_writer_object_end(w);

    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    }

    return ret;
}


int tc_iot_shadow_cmp_local(tc_iot_shadow_client * c, int property_id, void * src, void * dest) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_dest_offset = NULL;
    void * p_src_offset = NULL;
    int ret = 0;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_dest_offset = (char *)dest + p_prop->offset;
    p_src_offset = (char *)src + p_prop->offset;
    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            ret = memcmp( p_dest_offset, p_src_offset, sizeof(tc_iot_shadow_bool));
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %s -> %s", p_prop->name,
                        *(tc_iot_shadow_bool*)p_src_offset ? TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE,
                        *(tc_iot_shadow_bool*)p_dest_offset ? TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE
                        );
            }
            break;
        case TC_IOT_SHADOW_TYPE_NUMBER:
            ret = memcmp( p_dest_offset, p_src_offset, sizeof(tc_iot_shadow_number));
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %f -> %f", p_prop->name,
                        *(tc_iot_shadow_number*)p_src_offset,
                        *(tc_iot_shadow_number*)p_dest_offset
                        );
            }
            break;
        case TC_IOT_SHADOW_TYPE_ENUM:
            ret = memcmp( p_dest_offset, p_src_offset, sizeof(tc_iot_shadow_enum));
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %d -> %d", p_prop->name,
                        *(tc_iot_shadow_enum*)p_src_offset,
                        *(tc_iot_shadow_enum*)p_dest_offset
                        );
            }
            break;
        case TC_IOT_SHADOW_TYPE_INT:
            ret = memcmp( p_dest_offset, p_src_offset, sizeof(tc_iot_shadow_int));
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %d -> %d", p_prop->name,
                        *(tc_iot_shadow_int*)p_src_offset,
                        *(tc_iot_shadow_int*)p_dest_offset
                        );
            }
            break;
        case TC_IOT_SHADOW_TYPE_STRING:
            ret = strcmp( p_dest_offset, p_src_offset);
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %s -> %s", p_prop->name,
                        (tc_iot_shadow_string)p_src_offset,
                        (tc_iot_shadow_string)p_dest_offset
                        );
            }
            break;
        default:
            TC_IOT_LOG_ERROR("invalid data type=%d found", p_prop->type);
            return -1;
    }
    return ret;
}

int tc_iot_shadow_cmp_reported_with_local(tc_iot_shadow_client * c, int property_id) {

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    return tc_iot_shadow_cmp_local(c, property_id,
            c->p_shadow_config->p_reported_device_data,
            c->p_shadow_config->p_current_device_data
            );
}


int tc_iot_shadow_cmp_desired_with_local(tc_iot_shadow_client * c, int property_id) {

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    return tc_iot_shadow_cmp_local(c, property_id,
            c->p_shadow_config->p_desired_device_data,
            c->p_shadow_config->p_current_device_data
            );
}

void * tc_iot_shadow_copy_local_to_reported(tc_iot_shadow_client * c, int property_id) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_current = NULL;
    void * p_reported = NULL;

    IF_NULL_RETURN_DATA(c, p_reported);
    IF_NULL_RETURN_DATA(c->p_shadow_config, p_reported);
    IF_NULL_RETURN_DATA(c->p_shadow_config->properties, p_reported);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_current = (char *)c->p_shadow_config->p_current_device_data + p_prop->offset;
    p_reported = (char *)c->p_shadow_config->p_reported_device_data + p_prop->offset;

    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            return memcpy( p_reported, p_current, sizeof(tc_iot_shadow_bool));
        case TC_IOT_SHADOW_TYPE_NUMBER:
            return memcpy( p_reported, p_current, sizeof(tc_iot_shadow_number));
        case TC_IOT_SHADOW_TYPE_ENUM:
            return memcpy( p_reported, p_current, sizeof(tc_iot_shadow_enum));
        case TC_IOT_SHADOW_TYPE_STRING:
            return strcpy( p_reported, p_current);
        default:
            TC_IOT_LOG_ERROR("invalid data type=%d found", p_prop->type);
            return p_reported;
    }
}

void * tc_iot_shadow_save_to_cached(tc_iot_shadow_client * c, int property_id, const void * p_data, void * p_cache) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_dest = NULL;

    IF_NULL_RETURN_DATA(c, p_dest);
    IF_NULL_RETURN_DATA(c->p_shadow_config, p_dest);
    IF_NULL_RETURN_DATA(c->p_shadow_config->properties, p_dest);
    IF_NULL_RETURN_DATA(p_cache, p_dest);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_dest = (char *)p_cache + p_prop->offset;

    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            return memcpy( p_dest, p_data, sizeof(tc_iot_shadow_bool));
        case TC_IOT_SHADOW_TYPE_NUMBER:
            return memcpy( p_dest, p_data, sizeof(tc_iot_shadow_number));
        case TC_IOT_SHADOW_TYPE_ENUM:
            return memcpy( p_dest, p_data, sizeof(tc_iot_shadow_enum));
        case TC_IOT_SHADOW_TYPE_STRING:
            return strcpy( p_dest, p_data);
        default:
            TC_IOT_LOG_ERROR("invalid data type=%d found", p_prop->type);
            return p_dest;
    }
}

void * tc_iot_shadow_save_string_to_cached(tc_iot_shadow_client * c, int property_id, const void * p_data, int len, void * p_cache) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_dest = NULL;

    IF_NULL_RETURN_DATA(c, p_dest);
    IF_NULL_RETURN_DATA(c->p_shadow_config, p_dest);
    IF_NULL_RETURN_DATA(c->p_shadow_config->properties, p_dest);
    IF_NULL_RETURN_DATA(p_cache, p_dest);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_dest = (char *)p_cache + p_prop->offset;

    if (len >= p_prop->len) {
        TC_IOT_LOG_ERROR("source data too long len=%d, field %s max len=%d", len,p_prop->name, p_prop->len-1);
        return p_dest;
    }

    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_STRING:
            memcpy( p_dest, p_data, len);
            ((char *)(p_dest))[len] = '\0';
            return p_dest;
        default:
            TC_IOT_LOG_ERROR("invalid data type=%d found", p_prop->type);
            return p_dest;
    }
}

int tc_iot_shadow_report_property(tc_iot_shadow_client * c, int property_id, tc_iot_json_writer * w) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_current = NULL;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_current = (char *)c->p_shadow_config->p_current_device_data + p_prop->offset;
    /* p_reported = (char *)c->p_shadow_config->p_reported_device_data + p_prop->offset; */

    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            /* bool 类型数据，在数据模板中实际是以0/1表示。*/
            return tc_iot_json_writer_raw_data(w, p_prop->name, *(tc_iot_shadow_bool *)p_current?TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE);
        case TC_IOT_SHADOW_TYPE_NUMBER:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            return tc_iot_json_writer_decimal(w, p_prop->name, *(tc_iot_shadow_number *)p_current);
        case TC_IOT_SHADOW_TYPE_ENUM:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            return tc_iot_json_writer_int(w, p_prop->name, *(tc_iot_shadow_enum *)p_current);
        case TC_IOT_SHADOW_TYPE_INT:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            return tc_iot_json_writer_int(w, p_prop->name, *(tc_iot_shadow_int *)p_current);
        case TC_IOT_SHADOW_TYPE_STRING:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            return tc_iot_json_writer_string(w, p_prop->name, p_current);
        default:
            TC_IOT_LOG_ERROR("invalid data name=%s,type=%d found", p_prop->name, p_prop->type);
            return 0;
    }
}

int tc_iot_shadow_confirm_change(tc_iot_shadow_client * c, int property_id, tc_iot_json_writer * w) {
    tc_iot_shadow_property_def * p_prop = NULL;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    p_prop = &c->p_shadow_config->properties[property_id];
    return tc_iot_json_writer_null(w,p_prop->name);
}


int tc_iot_shadow_check_and_report(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        message_ack_handler callback, int timeout_ms, void * session_context, bool do_confirm) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    int sid_len;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;


    int ret = 0;
    int i = 0;
    int pos = 0;

    int desired_count = 0;
    int reported_count = 0;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    p_session = tc_iot_find_empty_session(c);
    if (!p_session) {
        TC_IOT_LOG_ERROR("no more empty session.");
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    }

    if (do_confirm) {
        tc_iot_json_writer_open(w, buffer, buffer_len);
        tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_DELETE);
    } else {
        tc_iot_json_writer_open(w, buffer, buffer_len);
        tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_UPDATE);
    }

    sid_len = _generate_session_id(&(p_session->sid[0]), TC_IOT_SESSION_ID_LEN+1, &(c->mqtt_client));
    if (sid_len <= 0) {
        TC_IOT_LOG_ERROR("generate session id failed: sid_len=%d", sid_len);
        memset(&(p_session->sid[0]), '0', TC_IOT_SESSION_ID_LEN);
        sid_len = TC_IOT_SESSION_ID_LEN;
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    } else {
        tc_iot_json_writer_object_begin(w ,"passthrough");
        
        tc_iot_json_writer_string(w ,"sid", &(p_session->sid[0]));
        tc_iot_json_writer_object_end(w);
    }

    if (do_confirm) {
        tc_iot_json_writer_uint(w ,TC_IOT_SHADOW_SEQUENCE_FIELD, c->shadow_seq);
    } else {
        // reported with no version
        // tc_iot_json_writer_uint(w ,"version", c->desired_version);
    }

    tc_iot_json_writer_object_begin(w ,"state");
    if (do_confirm) {
        tc_iot_json_writer_object_begin(w ,"desired");
        for (i = 0; i < c->p_shadow_config->property_total; ++i) {
            /* 清空 desired 数据 */
            if (TC_IOT_BIT_GET(c->desired_bits,i)) {
                /* 仅当本地状态和服务端在控制指令状态一致时，发送确认指令，清空 desired 数据。 */
                if (0 != tc_iot_shadow_cmp_desired_with_local(c, i)) {
                    TC_IOT_LOG_ERROR("device data %s change failure detected(local != desired)", tc_iot_shadow_get_property_name(c,i));
                    continue;
                }
                desired_count++;
                ret = tc_iot_shadow_confirm_change(c,i,w);
                if (ret <= 0) {
                    rc = TC_IOT_BUFFER_OVERFLOW;
                    goto exit;
                }
                TC_IOT_BIT_CLEAR(c->desired_bits, i);
            }
        }
        tc_iot_json_writer_object_end(w);

    } else {
        tc_iot_json_writer_object_begin(w ,"reported");
        for (i = 0; i < c->p_shadow_config->property_total; ++i) {
            /* 未上报过的数据，无条件做上报 */
            if (!TC_IOT_BIT_GET(c->reported_bits,i)) {
                reported_count++;

                ret = tc_iot_shadow_report_property(c, i, w);
                if (ret <= 0) {
                    rc = TC_IOT_BUFFER_OVERFLOW;
                    goto exit;
                }
                TC_IOT_BIT_SET(c->reported_bits,i);
            } else {
                /* 上报过的数据，则对于本地数据和已上报数据不一致的，才做上报 */
                if (tc_iot_shadow_cmp_reported_with_local(c, i) != 0) {
                    reported_count++;
                    ret = tc_iot_shadow_report_property(c, i, w);
                    if (ret <= 0) {
                        rc = TC_IOT_BUFFER_OVERFLOW;
                        goto exit;
                    }
                    TC_IOT_BIT_SET(c->reported_bits,i);
                }
            }
        }
        tc_iot_json_writer_object_end(w);
    }

    tc_iot_json_writer_object_end(w);

    if (desired_count <= 0 && reported_count <= 0) {
        TC_IOT_LOG_TRACE("No device data needed be reported.");
        rc = TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE;
        goto exit;
    }

    tc_iot_json_writer_object_end(w);
    pos += ret;
    if (pos >= buffer_len) {
        rc = TC_IOT_BUFFER_OVERFLOW;
        goto exit;
    }

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
    TC_IOT_LOG_TRACE("requesting with: %s", (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }

exit:
    if (rc != TC_IOT_SUCCESS) {
        tc_iot_release_session(p_session);
    }
    return rc;
}


