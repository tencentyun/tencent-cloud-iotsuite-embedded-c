#include "tc_iot_inc.h"

tc_iot_ota_state_item g_tc_iot_ota_state_items[] = {
    {"0","OTA Initialized"},
    {"1","OTA command received"},
    {"2","Firmware version check"},
    {"3","OTA firmware download"},
    {"4","OTA MD5 check"},
    {"5","Firmware upgrading"},
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

int tc_iot_ota_construct(tc_iot_ota_handler * ota_handler, tc_iot_mqtt_client * mqtt_client, 
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

void tc_iot_ota_destroy(tc_iot_ota_handler * ota_handler) {

    if (!ota_handler) {
        TC_IOT_LOG_ERROR("ota_handler is null");
        return ;
    }

    if (!ota_handler->p_mqtt_client) {
        TC_IOT_LOG_ERROR("ota_handler->p_mqtt_client is null");
        return ;
    }

    TC_IOT_LOG_TRACE("unsubscribing from topic:%s", ota_handler->sub_topic);
    tc_iot_mqtt_unsubscribe(ota_handler->p_mqtt_client, ota_handler->sub_topic);
    TC_IOT_LOG_TRACE("yielding for unsub ack");
    tc_iot_mqtt_yield(ota_handler->p_mqtt_client, 100);

    ota_handler->state = OTA_INITIALIZED;
    ota_handler->ota_id[0] = '\0';
    ota_handler->firmware_md5[0] = '\0';
    ota_handler->download_url[0] = '\0';
    ota_handler->version[0] = '\0';

    ota_handler->p_mqtt_client = NULL;
    ota_handler->sub_topic = NULL;
    ota_handler->pub_topic = NULL;
    TC_IOT_LOG_TRACE("OTA handler released");
}

int tc_iot_ota_format_message(tc_iot_ota_handler * ota_handler, char * buffer, int buffer_len, 
        tc_iot_ota_state_e state, const char * message, int percent) {
    tc_iot_ota_state_item * state_item = NULL;
    static int mid = 0;

    if (!buffer) {
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    state_item = tc_iot_ota_get_state_item(state);
    mid++;

    if (OTA_DOWNLOAD != state) {
        if (message) {
            return tc_iot_hal_snprintf(buffer, buffer_len, 
                    "{\"method\":\"%s\","
                    "\"passthrough\":{\"mid\":%d},"
                    "\"payload\":"
                    "{\"ota_id\":\"%s\",\"ota_code\":\"%s\",\"ota_status\":\"%s\",\"ota_message\":\"%s\"}"
                    "}"
                    ,
                    TC_IOT_OTA_METHOD_REPORT_UPGRADE,
                    mid,
                    ota_handler->ota_id,
                    state_item->code,
                    state_item->status,
                    message
                    );
        } else {
            return tc_iot_hal_snprintf(buffer, buffer_len, 
                    "{\"method\":\"%s\","
                    "\"passthrough\":{\"mid\":%d},"
                    "\"payload\":"
                    "{\"ota_id\":\"%s\",\"ota_code\":\"%s\",\"ota_status\":\"%s\",\"ota_message\":\"%s\"}"
                    "}"
                    ,
                    TC_IOT_OTA_METHOD_REPORT_UPGRADE,
                    mid,
                    ota_handler->ota_id,
                    state_item->code,
                    state_item->status,
                    ""
                    );
        }
    } else {
        return tc_iot_hal_snprintf(buffer, buffer_len, 
                "{\"method\":\"%s\","
                "\"passthrough\":{\"mid\":%d},"
                "\"payload\":"
                "{\"ota_id\":\"%s\",\"ota_code\":\"%s\",\"ota_status\":\"%s\",\"ota_message\":\"%d\"}"
                "}"
                ,
                TC_IOT_OTA_METHOD_REPORT_UPGRADE,
                mid,
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

int tc_iot_ota_report_upgrade(tc_iot_ota_handler * ota_handler, tc_iot_ota_state_e state, char * message, int percent) {
    char buffer[TC_IOT_REPORT_UPGRADE_MSG_LEN];
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

    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_UPGRADE_MSG_LEN]", sizeof(buffer), strlen(buffer));

    return tc_iot_ota_send_message(ota_handler, buffer);
}

static int tc_iot_ota_report_firm_info(tc_iot_ota_handler * ota_handler, char * buffer, int buffer_len, va_list p_args) {
    int rc ;
    tc_iot_mqtt_message pubmsg;

    int ret = 0;
    int i = 0;
    const char * info_name = NULL;
    const char * info_value = NULL;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    IF_NULL_RETURN(ota_handler, TC_IOT_NULL_POINTER);

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", TC_IOT_OTA_METHOD_REPORT_FIRM);
    tc_iot_json_writer_object_begin(w ,"payload");

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

int tc_iot_ota_report_firm(tc_iot_ota_handler * ota_handler, ...) {
    char buffer[TC_IOT_REPORT_FIRM_MSG_LEN];
    int buffer_len = sizeof(buffer);

    int ret = 0;
    va_list p_args;

    va_start(p_args, ota_handler);
    ret = tc_iot_ota_report_firm_info(ota_handler, buffer, buffer_len, p_args);
    if (ret != TC_IOT_SUCCESS) {
        TC_IOT_LOG_ERROR("[c-s]update_firm_info failed(%d): %s", ret, buffer);
    }
    va_end( p_args);
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_FIRM_MSG_LEN]", sizeof(buffer), strlen(buffer));
    return ret;
}

/* 版本号校验规则：固件版本号的组成 为 “客户自定义部分” + “系统强制部分”； */
/* 系统强制部分 以大写字母V开头，然后后面可以跟只允许一个小数点的数字。 */
/* 设备端根据这个V后面的数字大小去比对版本，设备只允许向高版本升级；  */
/* 例如某个客户是一个基于某ESP8266模组的固件程序，则命名为ESP8266V1.0，ESP8266V11.198 都可以通过， */
/* 系统只需验证V后面必须是一个数值型数字。 */
/* 版本数字比较：V1.9 与V1.10 哪个大，本系统约定1.9 > 1.10，按V后面的数值大小比较大小 */
double tc_iot_ota_find_version_number(const char * version) {
    const char * pos = NULL;
    double num = 0;
    char chr = 0;

    if (!version) {
        return 0;
    }

    pos = version + strlen(version) - 1;
    while (pos >= version) {
        chr = *pos;
        if ('V' == chr || 'v' == chr) {
            break;
        }
        pos--;
    }

    num = atof(pos+1);

    return num;
}

bool tc_iot_ota_version_larger(const char * mine_version, const char * their_version) {
    double mine_no = 0;
    double their_no = 0;

    if (!mine_version || !their_version) {
        return false;
    }
    mine_no = tc_iot_ota_find_version_number(mine_version);
    if (mine_no == 0) {
        return false;
    }

    their_no = tc_iot_ota_find_version_number(their_version);
    if (their_no == 0) {
        return false;
    }
    
    return mine_no > their_no;
}

