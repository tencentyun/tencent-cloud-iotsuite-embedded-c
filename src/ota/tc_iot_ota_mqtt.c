#include "tc_iot_inc.h"

tc_iot_ota_state_item g_tc_iot_ota_state_items[] = {
    {"00","OTA Initialized"},
    {"01","OTA command received"},
    {"02","Firmware version checking"},
    {"03","OTA firmware downloading"},
    {"04","OTA MD5 checking"},
    {"05","OTA start upgrade"},
    {"06","OTA upgrading"},
};


tc_iot_ota_state_item * tc_iot_ota_get_state_item(tc_iot_ota_state_e state) {
    if (state < OTA_MAX_STATE) {
        return &g_tc_iot_ota_state_items[state];
    } else {
        TC_IOT_LOG_ERROR("state=%d invalid", state);
        return NULL;
    }
}

void _tc_iot_on_ota_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    TC_IOT_LOG_TRACE("[s->c] %s\n", (char*)message->payload);
}

int tc_iot_ota_set_ota_id(tc_iot_ota_handler * ota_handler, const char * ota_id) {
    int ota_id_len = 0;

    if (!ota_handler) {
        TC_IOT_LOG_ERROR("ota_handler is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!ota_id) {
        TC_IOT_LOG_ERROR("ota_id is null");
        return TC_IOT_NULL_POINTER;
    }
    
    tc_iot_hal_snprintf(ota_handler->ota_id, sizeof(ota_handler->ota_id), "%s", ota_id);
    return TC_IOT_SUCCESS;
}

int tc_iot_ota_init(tc_iot_ota_handler * ota_handler, tc_iot_mqtt_client * mqtt_client, 
        const char * product_id, const char * device_name) {
    int ret = 0;
    if (!ota_handler) {
        TC_IOT_LOG_ERROR("ota_handler is null");
        return TC_IOT_NULL_POINTER;
    }

    ota_handler->state = OTA_INITIALIZED;
    ota_handler->ota_id[0] = '\0';
    ota_handler->firmware_md5[0] = '\0';
    ota_handler->firmware_url[0] = '\0';

    ota_handler->p_mqtt_client = mqtt_client;
    tc_iot_hal_snprintf(ota_handler->sub_topic, sizeof(ota_handler->sub_topic), "ota/get/%s/%s", product_id, device_name);
    tc_iot_hal_snprintf(ota_handler->pub_topic, sizeof(ota_handler->pub_topic), "ota/update/%s/%s", product_id, device_name);

    ret = tc_iot_mqtt_subscribe(ota_handler->p_mqtt_client, ota_handler->sub_topic, TC_IOT_QOS1, _tc_iot_on_ota_message_received,ota_handler);

    return ret;
}

int tc_iot_ota_format_message(tc_iot_ota_handler * ota_handler, char * buffer, int buffer_len, 
        tc_iot_ota_state_e state, const char * message, int percent) {
    tc_iot_ota_state_item * state_item = NULL;

    if (!buffer) {
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    state_item = tc_iot_ota_get_state_item(state);

    if (OTA_DOWNLOAD != state) {
        if (message) {
            return tc_iot_hal_snprintf(buffer, buffer_len, 
                    /* "{\"method\":\"ota_report\",\"payload\":" */
                    "{\"ota_id\":\"%s\",\"ota_code\":\"%s\",\"ota_status\":\"%s\",\"ota_message\":\"%s\"}",
                    ota_handler->ota_id,
                    state_item->code,
                    state_item->status,
                    message
                    );
        } else {
            return tc_iot_hal_snprintf(buffer, buffer_len, 
                    /* "{\"method\":\"ota_report\",\"payload\":" */
                    "{\"ota_id\":\"%s\",\"ota_code\":\"%s\",\"ota_status\":\"%s\"}",
                    ota_handler->ota_id,
                    state_item->code,
                    state_item->status
                    );
        }
    } else {
        return tc_iot_hal_snprintf(buffer, buffer_len, 
                /* "{\"method\":\"ota_report\",\"payload\":" */
                "{\"ota_id\":\"%s\",\"ota_code\":\"%s\",\"ota_status\":\"%s\",\"ota_message\":\"%d\"}",
                ota_handler->ota_id,
                state_item->code,
                state_item->status,
                percent
                );
    }

}

int tc_iot_ota_send_message(tc_iot_ota_handler * ota_handler, char * message) {
    tc_iot_mqtt_message pubmsg;
    if (!ota_handler) {
        TC_IOT_LOG_ERROR("ota_handler is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!ota_handler->p_mqtt_client) {
        TC_IOT_LOG_ERROR("ota_handler->p_mqtt_client is null");
        return TC_IOT_NULL_POINTER;
    }
    
    memset(&pubmsg, '\0', sizeof(pubmsg));
    pubmsg.payload = message;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;

    TC_IOT_LOG_TRACE("report: %s|%s", ota_handler->pub_topic, message);
    return tc_iot_mqtt_publish(ota_handler->p_mqtt_client, ota_handler->pub_topic, &pubmsg);
}

int tc_iot_ota_report(tc_iot_ota_handler * ota_handler, tc_iot_ota_state_e state, char * message, int percent) {
    char buffer[128];
    int ret;

    ret = tc_iot_ota_format_message(ota_handler, buffer, sizeof(buffer), state, message, percent);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("tc_iot_ota_format_message ret=%d", ret);
        return ret;
    }

    if (ret >= sizeof(buffer)) {
        TC_IOT_LOG_ERROR("tc_iot_ota_format_message ret=%d, buffer overflow", ret);
        return TC_IOT_BUFFER_OVERFLOW;
    }


    return tc_iot_ota_send_message(ota_handler, buffer);
}

int tc_iot_ota_report_firm(tc_iot_ota_handler * ota_handler, ...) {
    char buffer[512];
    int buffer_len = sizeof(buffer);

    int ret = 0;
    va_list p_args;

    va_start(p_args, ota_handler);
    ret = tc_iot_ota_update_firm_info(ota_handler, buffer, buffer_len, p_args);
    if (ret == TC_IOT_SUCCESS) {
        TC_IOT_LOG_TRACE("[c-s]update_firm_info: %s", buffer);
    } else {
        TC_IOT_LOG_TRACE("[c-s]update_firm_info failed(%d): %s", ret, buffer);
    }
    va_end( p_args);
    return ret;
}

int tc_iot_ota_update_firm_info(tc_iot_ota_handler * ota_handler, char * buffer, int buffer_len, va_list p_args) {
    char *pub_topic ;
    int rc ;
    tc_iot_mqtt_message pubmsg;

    int ret = 0;
    int i = 0;
    int pos = 0;
    const char * info_name = NULL;
    const char * info_value = NULL;

    IF_NULL_RETURN(ota_handler, TC_IOT_NULL_POINTER);

    ret = tc_iot_ota_doc_pack_start(buffer+pos, buffer_len-pos, NULL, TC_IOT_SESSION_ID_LEN+1, TC_IOT_MQTT_METHOD_UPDATE_FIRM, ota_handler);
    if (ret <= 0) {
        rc = TC_IOT_BUFFER_OVERFLOW;
        goto exit;
    }
    pos += ret;

    ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, ",\"state\":{");
    if (ret <= 0) {
        rc = TC_IOT_BUFFER_OVERFLOW;
        goto exit;
    }
    pos += ret;

    for(i = 0; i < TC_IOT_MAX_FIRM_INFO_COUNT; i++) {
        info_name = va_arg (p_args, const char *);
        if (info_name == NULL) {
            break;
        }

        info_value = va_arg (p_args, const char *);
        if (info_value == NULL) {
            break;
        }

        if (strlen(info_name) > TC_IOT_MAX_FIRM_INFO_NAME_LEN) {
            TC_IOT_LOG_ERROR("firm name too long:%s", info_name);
            rc = TC_IOT_FIRM_INFO_NAME_TOO_LONG;
            goto exit;
        }

        if (strlen(info_value) > TC_IOT_MAX_FIRM_INFO_VALUE_LEN) {
            TC_IOT_LOG_ERROR("firm value too long:%s", info_value);
            rc = TC_IOT_FIRM_INFO_VALUE_TOO_LONG;
            goto exit;
        }

        if (i == 0) {
            ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, "\"");
            if (ret <= 0) {
                rc = TC_IOT_BUFFER_OVERFLOW;
                goto exit;
            }
            pos += ret;
        } else {
            ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, ",\"");
            if (ret <= 0) {
                rc = TC_IOT_BUFFER_OVERFLOW;
                goto exit;
            }
            pos += ret;
        }

        ret = tc_iot_json_escape(buffer + pos, buffer_len-pos, info_name, strlen(info_name));
        if (ret <= 0) {
            rc = TC_IOT_BUFFER_OVERFLOW;
            goto exit;
        }
        pos += ret;


        ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, "\":\"");
        if (ret <= 0) {
            rc = TC_IOT_BUFFER_OVERFLOW;
            goto exit;
        }
        pos += ret;

        ret = tc_iot_json_escape(buffer + pos, buffer_len-pos, info_value, strlen(info_value));
        if (ret <= 0) {
            rc = TC_IOT_BUFFER_OVERFLOW;
            goto exit;
        }
        pos += ret;

        ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, "\"");
        if (ret <= 0) {
            rc = TC_IOT_BUFFER_OVERFLOW;
            goto exit;
        }
        pos += ret;
    }

    ret = tc_iot_hal_snprintf(buffer + pos, buffer_len-pos, "}");
    if (ret <= 0) {
        rc = TC_IOT_BUFFER_OVERFLOW;
        goto exit;
    }
    pos += ret;

    ret = tc_iot_ota_doc_pack_end(buffer+pos, buffer_len-pos, ota_handler);
    if (ret <= 0) {
        rc = TC_IOT_BUFFER_OVERFLOW;
        goto exit;
    }
    pos += ret;

    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;

    pub_topic = ota_handler->pub_topic;
    rc = tc_iot_mqtt_client_publish(ota_handler->p_mqtt_client, pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    } else {
        TC_IOT_LOG_TRACE("publish success:%s|%s", pub_topic, buffer);
    }
exit:
    /* if (rc != TC_IOT_SUCCESS) { */
    /* } */
    return rc;
}

int tc_iot_ota_doc_pack_start(char *buffer, int buffer_len,
                                 char * session_id, int session_id_len,
                                 const char * method,
                                 tc_iot_ota_handler *c) {
    int ret;
    int buffer_used = 0;
    int sid_len = 0;

    ret = tc_iot_hal_snprintf(buffer + buffer_used, buffer_len, "{\"method\":\"%s\"", method);

    buffer_used += ret;
    return buffer_used;
}

int tc_iot_ota_doc_pack_end(char *buffer, int buffer_len, tc_iot_ota_handler * ota_handler) {
    int ret;
    int buffer_used = 0;

    ret = tc_iot_hal_snprintf(buffer, buffer_len, "}");

    buffer_used += ret;
    if (buffer_used < buffer_len) {
        buffer[buffer_used] = '\0';
    }

    return buffer_used;
}

