#include "tc_iot_export.h"

 /* 长度需要足够存放：
  * 1) 2^64 的十进制数据值。
  * 2) true or false 布尔字符串。
  * 3) method 字段数据: get, update, update_firm_info, report_firm_info, reply */
#define TC_IOT_MAX_FIELD_LEN  22


/* 影子数据 Client  */
tc_iot_shadow_client g_tc_iot_shadow_client;

tc_iot_shadow_client * tc_iot_get_shadow_client(void) {
    return &g_tc_iot_shadow_client;
}

/**
 * @brief _tc_iot_get_message_ack_callback shadow_get 回调函数
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void _tc_iot_get_message_ack_callback(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context) {

    tc_iot_mqtt_message* message = NULL;

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            LOG_ERROR("request timeout");
        }
        return;
    }

    message = md->message;
    tc_iot_device_on_message_received(md);
}

/**
 * @brief _tc_iot_report_message_ack_callback shadow_update 上报消息回调
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void _tc_iot_report_message_ack_callback(tc_iot_command_ack_status_e ack_status, 
        tc_iot_message_data * md , void * session_context) {
    tc_iot_mqtt_message* message = NULL;

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            LOG_ERROR("request timeout");
        } else {
            LOG_ERROR("request return ack_status=%d", ack_status);
        }
        return;
    }

    message = md->message;
    LOG_TRACE("[s->c] %s", (char*)message->payload);
}


/**
 * @brief _tc_iot_sync_shadow_property 根据服务端下发的影子数据，同步到本地设备状态数据，并进
 * 行上报。
 *
 * @param property_count 设备状态字段数
 * @param properties 设备状态数据
 * @param reported 同步数据类型，为 true 表示同步 reported 数据，为
 * false 表示同步 current 数据。
 * @param doc_start reported or desired 数据起始位置。
 * @param json_token json token 数组起始位置
 * @param tok_count 有效 json token 数量
 *
 * @return 
 */
int _tc_iot_sync_shadow_property(tc_iot_shadow_client * p_shadow_client, 
        int property_total, tc_iot_shadow_property_def * properties, 
        const char * doc_start, jsmntok_t * json_token, int tok_count) {
    int i,j;
    jsmntok_t  * key_tok = NULL;
    jsmntok_t  * val_tok = NULL;
    char field_buf[TC_IOT_MAX_FIELD_LEN];
    int field_len = sizeof(field_buf);
    tc_iot_shadow_number new_number = 0;
    tc_iot_shadow_bool new_bool = 0;
    tc_iot_shadow_enum new_enum = 0;
    int  key_len = 0, val_len = 0;
    const char * key_start;
    const char * val_start;
    tc_iot_shadow_property_def * p_prop = NULL;
    void  * ptr = NULL;
    tc_iot_event_message event_msg;

    if (!properties) {
        LOG_ERROR("properties is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!doc_start) {
        LOG_ERROR("doc_start is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!json_token) {
        LOG_ERROR("json_token is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!tok_count) {
        LOG_ERROR("tok_count is invalid");
        return TC_IOT_INVALID_PARAMETER;
    }

    memset(field_buf, 0, sizeof(field_buf));

    for (i = 0; i < tok_count/2; i++) {
        /* 位置 0 是object对象，所以要从位置 1 开始取数据*/
        /* 2*i+1 为 key 字段，2*i + 2 为 value 字段*/
        key_tok = &(json_token[2*i + 1]);
        key_start = doc_start + key_tok->start;
        key_len = key_tok->end - key_tok->start;

        val_tok = &(json_token[2*i + 2]);
        val_start = doc_start + val_tok->start;
        val_len = val_tok->end - val_tok->start;
        for(j = 0; j < property_total; j++) {
            p_prop = &properties[j];
            if (strncmp(p_prop->name, key_start, key_len) == 0)  {
                if (p_prop->type == TC_IOT_SHADOW_TYPE_BOOL) {
                    strncpy(field_buf, val_start, val_len);
                    field_buf[val_len] = '\0';
                    new_bool = (strncmp(TC_IOT_JSON_TRUE, field_buf, val_len) == 0);
                    ptr = &new_bool;
                    LOG_TRACE("state change:[%s=%s]", p_prop->name, (*(tc_iot_shadow_bool *) ptr)?"true":"false");
                } else if (p_prop->type == TC_IOT_SHADOW_TYPE_NUMBER) {
                    strncpy(field_buf, val_start, val_len);
                    field_buf[val_len] = '\0';
                    new_number = atoi(field_buf);
                    ptr = &new_number;
                    LOG_TRACE("state change:[%s=%d]", p_prop->name, (*(tc_iot_shadow_number *) ptr));
                } else if (p_prop->type == TC_IOT_SHADOW_TYPE_ENUM) {
                    strncpy(field_buf, val_start, val_len);
                    field_buf[val_len] = '\0';
                    new_enum = atoi(field_buf);
                    ptr = &new_enum;
                    LOG_TRACE("state change:[%s|%d]", p_prop->name, (*(tc_iot_shadow_enum *) ptr));
                } else {
                    LOG_ERROR("%s type=%d invalid.", p_prop->name, p_prop->type);
                    continue;
                }
                
                if (p_shadow_client->p_shadow_config->event_notify) {
                    event_msg.event = TC_IOT_SHADOW_EVENT_SERVER_CONTROL;
                    event_msg.data = ptr;
                    p_shadow_client->p_shadow_config->event_notify(&event_msg, p_shadow_client, p_prop);
                }
            }
        }
    }
    return 0;
}



/**
 * @brief tc_iot_device_on_message_received 数据回调，处理 shadow_get 获取最新状态，或
 * 者影子服务推送的最新控制指令数据。
 *
 * @param md 影子数据消息
 */
void tc_iot_device_on_message_received(tc_iot_message_data* md) {
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    char field_buf[TC_IOT_MAX_FIELD_LEN];
    int field_index = 0;
    const char * reported_start = NULL;
    int reported_len = 0;
    const char * desired_start = NULL;
    int desired_len = 0;
    int ret = 0;
    tc_iot_event_message event_msg;
    tc_iot_shadow_client * p_shadow_client = tc_iot_get_shadow_client();

    memset(field_buf, 0, sizeof(field_buf));

    tc_iot_mqtt_message* message = md->message;
    LOG_TRACE("[s->c] %s", (char*)message->payload);

    /* 有效性检查 */
    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        return ;
    }

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "method", field_buf, sizeof(field_buf));
    if (field_index <= 0 ) {
        LOG_ERROR("field method not found in JSON: %s", (char*)message->payload);
        return ;
    }

    if (strncmp(TC_IOT_MQTT_METHOD_CONTROL, field_buf, strlen(field_buf)) == 0) {
        LOG_TRACE("Control data receved.");
    } else if (strncmp(TC_IOT_MQTT_METHOD_REPLY, field_buf, strlen(field_buf)) == 0) {
        LOG_TRACE("Reply pack recevied.");
    } else if (strncmp(TC_IOT_MQTT_METHOD_REPORT_FIRM, field_buf, strlen(field_buf)) == 0) {
        LOG_TRACE("request report firm info.");
        if (p_shadow_client->p_shadow_config->event_notify) {
            event_msg.event = TC_IOT_SHADOW_EVENT_REQUEST_REPORT_FIRM;
            event_msg.data = NULL;
            p_shadow_client->p_shadow_config->event_notify(&event_msg, p_shadow_client, NULL);
        } else {
            LOG_TRACE("no event_notify callback, skip report firm.");
        }
        return;
    }

    /* 检查 reported 字段是否存在 */
    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, 
            "payload.state.reported", NULL, 0);
    if (field_index <= 0 ) {
        /* LOG_TRACE("payload.state.reported not found"); */
    } else {
        reported_start = (const char *)message->payload + json_token[field_index].start;
        reported_len = json_token[field_index].end - json_token[field_index].start;
        LOG_TRACE("payload.state.reported found:%.*s", reported_len, reported_start);
    }

    /* 检查 desired 字段是否存在 */
    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, 
            "payload.state.desired", NULL, 0);
    if (field_index <= 0 ) {
        /* LOG_TRACE("payload.state.desired not found"); */
    } else {
        desired_start = (const char *)message->payload + json_token[field_index].start;
        desired_len = json_token[field_index].end - json_token[field_index].start;
        LOG_TRACE("payload.state.desired found:%.*s", desired_len, desired_start);
    }

    /* 根据控制台或者 APP 端的指令，设定设备状态 */
    if (desired_start) {
        ret = tc_iot_json_parse(desired_start,desired_len, json_token, TC_IOT_ARRAY_LENGTH(json_token));
        if (ret <= 0) {
            return ;
        }
        LOG_TRACE("---synchronizing desired status---");
        _tc_iot_sync_shadow_property(
                p_shadow_client,
                p_shadow_client->p_shadow_config->property_total,
                p_shadow_client->p_shadow_config->properties, 
                desired_start, json_token, ret);
        LOG_TRACE("---synchronizing desired status success---");
    }
}

int tc_iot_shadow_update_reported_propeties(int property_count, ...) {
    char buffer[512];
    int buffer_len = sizeof(buffer);

    int ret = 0;
    va_list p_args;
    tc_iot_shadow_client* p_shadow_client = tc_iot_get_shadow_client();

    va_start(p_args, property_count);
    ret = tc_iot_shadow_update_state(p_shadow_client, buffer, buffer_len,  
            _tc_iot_report_message_ack_callback, p_shadow_client->mqtt_client.command_timeout_ms, NULL,
            "reported", property_count, p_args);
    va_end( p_args);
    return ret;
}

int tc_iot_shadow_update_desired_propeties(int property_count, ...) {
    char buffer[512];
    int buffer_len = sizeof(buffer);
    tc_iot_shadow_client* p_shadow_client = tc_iot_get_shadow_client();

    int ret = 0;
    va_list p_args;

    va_start(p_args, property_count);
    ret = tc_iot_shadow_update_state(p_shadow_client, buffer, buffer_len,  
            _tc_iot_report_message_ack_callback, p_shadow_client->mqtt_client.command_timeout_ms, NULL,
            "desired", property_count, p_args);
    va_end( p_args);
    return ret;
}

int tc_iot_report_firm(int info_count, ...) {
    char buffer[512];
    int buffer_len = sizeof(buffer);

    int ret = 0;
    va_list p_args;
    tc_iot_shadow_client* p_shadow_client = tc_iot_get_shadow_client();

    va_start(p_args, info_count);
    ret = tc_iot_shadow_update_firm_info(p_shadow_client, buffer, buffer_len,  
            _tc_iot_report_message_ack_callback, p_shadow_client->mqtt_client.command_timeout_ms, NULL,
            info_count, p_args);
    LOG_TRACE("[c-s]update_firm_info: %s", buffer);
    va_end( p_args);
    return ret;
}

int tc_iot_server_init(tc_iot_shadow_config * p_client_config) {
    int ret = 0;
    char buffer[128];
    int buffer_len = sizeof(buffer);
    tc_iot_shadow_client* p_shadow_client = tc_iot_get_shadow_client();

    /* 初始化 shadow client */
    LOG_INFO("constructing mqtt shadow client.");
    ret = tc_iot_shadow_construct(p_shadow_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        return ret;
    }

    LOG_INFO("construct mqtt shadow client success.");
    LOG_INFO("yield waiting for server push.");
    /* 执行 yield 收取影子服务端前序指令消息，清理历史状态。 */
    tc_iot_shadow_yield(p_shadow_client, 200);
    LOG_INFO("yield waiting for server finished.");

    /* 通过get操作主动获取服务端影子设备状态，以便设备端同步更新至最新状态*/
    ret = tc_iot_shadow_get(p_shadow_client, buffer, buffer_len, _tc_iot_get_message_ack_callback, 
            p_shadow_client->mqtt_client.command_timeout_ms, p_shadow_client);
    LOG_TRACE("[c->s] shadow_get%s", buffer);

    return ret;
}

int tc_iot_server_loop(int yield_timeout) {
    tc_iot_shadow_client* p_shadow_client = tc_iot_get_shadow_client();
    return tc_iot_shadow_yield(p_shadow_client, yield_timeout);
}

int tc_iot_server_destroy(void) {
    tc_iot_shadow_client* p_shadow_client = tc_iot_get_shadow_client();

    LOG_TRACE("Stopping");
    tc_iot_shadow_destroy(p_shadow_client);
    LOG_TRACE("Exit success.");
    return 0;
}

