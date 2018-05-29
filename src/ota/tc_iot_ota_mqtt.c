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
        const char * sub_topic, const char * pub_topic, message_handler ota_msg_callback) {
    int ret = 0;
    if (!ota_handler) {
        TC_IOT_LOG_ERROR("ota_handler is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!ota_msg_callback) {
        TC_IOT_LOG_ERROR("ota_msg_callback is null");
        return TC_IOT_NULL_POINTER;
    }

    ota_handler->state = OTA_INITIALIZED;
    ota_handler->ota_id[0] = '\0';
    ota_handler->firmware_md5[0] = '\0';
    ota_handler->download_url[0] = '\0';
    ota_handler->version[0] = '\0';

    ota_handler->p_mqtt_client = mqtt_client;
    ota_handler->sub_topic = sub_topic;
    ota_handler->pub_topic = pub_topic;

    ret = tc_iot_mqtt_subscribe(ota_handler->p_mqtt_client, ota_handler->sub_topic, TC_IOT_QOS1, ota_msg_callback, ota_handler);

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
    char buffer[200];
    int buffer_len = sizeof(buffer);

    int ret = 0;
    va_list p_args;

    va_start(p_args, ota_handler);
    ret = tc_iot_ota_update_firm_info(ota_handler, buffer, buffer_len, p_args);
    if (ret != TC_IOT_SUCCESS) {
        TC_IOT_LOG_ERROR("[c-s]update_firm_info failed(%d): %s", ret, buffer);
    }
    va_end( p_args);
    return ret;
}

int tc_iot_ota_update_firm_info(tc_iot_ota_handler * ota_handler, char * buffer, int buffer_len, va_list p_args) {
    int rc ;
    tc_iot_mqtt_message pubmsg;

    int ret = 0;
    int i = 0;
    int pos = 0;
    const char * info_name = NULL;
    const char * info_value = NULL;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    IF_NULL_RETURN(ota_handler, TC_IOT_NULL_POINTER);

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_UPDATE_FIRM);
    tc_iot_json_writer_object_begin(w ,"state");

    for(i = 0; i < TC_IOT_MAX_FIRM_INFO_COUNT; i++) {
        info_name = va_arg (p_args, const char *);
        if (info_name == NULL) {
            break;
        }

        info_value = va_arg (p_args, const char *);
        if (info_value == NULL) {
            break;
        }
        tc_iot_json_writer_string(w, info_name, info_value);
    }

    tc_iot_json_writer_object_end(w);
    ret = tc_iot_json_writer_close(w);
    if (ret <= 0) {
        rc = TC_IOT_BUFFER_OVERFLOW;
        goto exit;
    }

    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;

    rc = tc_iot_mqtt_client_publish(ota_handler->p_mqtt_client, ota_handler->pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    } else {
        TC_IOT_LOG_TRACE("[c->s]:%s|%s", ota_handler->pub_topic, buffer);
    }
exit:
    /* if (rc != TC_IOT_SUCCESS) { */
    /* } */
    return rc;
}

