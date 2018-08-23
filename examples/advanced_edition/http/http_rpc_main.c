#include "tc_iot_export.h"
#include "tc_iot_device_config.h"


#define TC_IOT_PROP_param_bool 0
#define TC_IOT_PROP_param_enum 1
#define TC_IOT_PROP_param_number 2
#define TC_IOT_PROP_param_string 3

typedef struct _tc_iot_shadow_local_data {
    tc_iot_shadow_bool param_bool;
    tc_iot_shadow_enum param_enum;
    tc_iot_shadow_number param_number;
    char param_string[50+1];
}tc_iot_shadow_local_data;

// 数据模板字段，变量名最大长度
#define MAX_TEMPLATE_KEY_LEN   20
// 数据模板字段取值最大长度
#define MAX_VAL_BUFFER_LEN     50

extern void parse_command(tc_iot_device_info * p_device, int argc, char ** argv);
tc_iot_shadow_local_data g_local_data = {
    false,
    0,
    0,
    ""
};

tc_iot_device_info g_device_info = {
    /* device info*/
    TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
    TC_IOT_CONFIG_DEVICE_NAME, "",
};

int _process_desired( const char * doc_start, jsmntok_t * json_token, int tok_count, 
        unsigned char * p_desired_bits, tc_iot_shadow_local_data * p_local_data) {
    int i;
    jsmntok_t  * key_tok = NULL;
    jsmntok_t  * val_tok = NULL;
    char val_buf[MAX_VAL_BUFFER_LEN+1];
    char key_buf[MAX_TEMPLATE_KEY_LEN+1];
    int  key_len = 0, val_len = 0;
    const char * key_start;
    const char * val_start;

    if (!doc_start) {
        TC_IOT_LOG_ERROR("doc_start is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!json_token) {
        TC_IOT_LOG_ERROR("json_token is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!tok_count) {
        TC_IOT_LOG_ERROR("tok_count is invalid");
        return TC_IOT_INVALID_PARAMETER;
    }

    memset(val_buf, 0, sizeof(val_buf));
    memset(key_buf, 0, sizeof(key_buf));

    for (i = 0; i < tok_count/2; i++) {
        /* 位置 0 是object对象，所以要从位置 1 开始取数据*/
        /* 2*i+1 为 key 字段，2*i + 2 为 value 字段*/
        key_tok = &(json_token[2*i + 1]);
        key_start = doc_start + key_tok->start;
        key_len = key_tok->end - key_tok->start;
        if (key_len > MAX_TEMPLATE_KEY_LEN) {
            memcpy(key_buf, key_start, MAX_TEMPLATE_KEY_LEN);
            key_buf[MAX_TEMPLATE_KEY_LEN] = '\0';
            TC_IOT_LOG_ERROR("key=%s... length oversize.", key_buf);
            continue;
        }

        memcpy(key_buf, key_start, key_len);
        key_buf[key_len] = '\0';

        val_tok = &(json_token[2*i + 2]);
        val_start = doc_start + val_tok->start;
        val_len = val_tok->end - val_tok->start;
        if (val_len > TC_IOT_MAX_FIELD_LEN) {
            memcpy(val_buf, val_start, MAX_VAL_BUFFER_LEN);
            val_buf[MAX_VAL_BUFFER_LEN] = '\0';
            TC_IOT_LOG_ERROR("val=%s... length oversize.", val_buf);
            continue;
        }

        memcpy(val_buf, val_start, val_len);
        val_buf[val_len] = '\0';
        
        if (strcmp("param_bool", key_buf) == 0 ) {
            TC_IOT_BIT_SET(p_desired_bits, TC_IOT_PROP_param_bool);
            TC_IOT_LOG_TRACE("desired field: %s=%s->%s", key_buf, p_local_data->param_bool?"true":"false", val_buf);
            p_local_data->param_bool = (val_buf[0] != 'f') && (val_buf[0] != '0');
        } else if (strcmp("param_enum", key_buf) == 0 ) {
            TC_IOT_BIT_SET(p_desired_bits, TC_IOT_PROP_param_enum);
            TC_IOT_LOG_TRACE("desired field: %s=%d->%s", key_buf, p_local_data->param_enum, val_buf);
            p_local_data->param_enum = atoi(val_buf);
        } else if (strcmp("param_number", key_buf) == 0 ) {
            TC_IOT_BIT_SET(p_desired_bits, TC_IOT_PROP_param_number);
            TC_IOT_LOG_TRACE("desired field: %s=%f->%s", key_buf, p_local_data->param_number, val_buf);
            p_local_data->param_number = atof(val_buf);
        } else if (strcmp("param_string", key_buf) == 0 ) {
            TC_IOT_BIT_SET(p_desired_bits, TC_IOT_PROP_param_string);
            TC_IOT_LOG_TRACE("desired field: %s=%s->%s", key_buf, p_local_data->param_string, val_buf);
            strcpy(p_local_data->param_string, val_buf);
        } else {
            TC_IOT_LOG_ERROR("unknown desired field: %s=%s", key_buf, val_buf);
            continue;
        }
    }
    return TC_IOT_SUCCESS;
}

int check_and_process_desired(unsigned char * p_desired_bits, tc_iot_shadow_local_data * p_local_data, const char * message) {
    const char * desired_start = NULL;
    int desired_len = 0;
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    int field_index = 0;
    int ret = 0;
    int token_count = 0;

    /* 有效性检查 */
    ret = tc_iot_json_parse(message, strlen(message), json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("tc_iot_json_parse ret=%d", ret);
        return ret;
    }

    token_count = ret;
    tc_iot_mem_usage_log("json_token[TC_IOT_MAX_JSON_TOKEN_COUNT]", sizeof(json_token), sizeof(json_token[0])*token_count);

    /* 检查 desired 字段是否存在 */
    field_index = tc_iot_json_find_token(message, json_token, token_count, "payload.state.desired", NULL, 0);
    if (field_index <= 0 ) {
        TC_IOT_LOG_TRACE("payload.state.desired not found, ret=%d", field_index);
    } else {
        desired_start = message + json_token[field_index].start;
        desired_len = json_token[field_index].end - json_token[field_index].start;
        TC_IOT_LOG_TRACE("payload.state.desired found:%s", tc_iot_log_summary_string(desired_start, desired_len));
        /* 根据控制台或者 APP 端的指令，设定设备状态 */
        if (desired_start) {
            ret = tc_iot_json_parse(desired_start, desired_len, json_token, token_count);
            if (ret <= 0) {
                return TC_IOT_FAILURE;
            }
            _process_desired( desired_start, json_token, ret, p_desired_bits, p_local_data);
        }
    }

    return TC_IOT_SUCCESS;
}

int do_rpc_get(char * result, int result_len, tc_iot_device_info * p_device_info, bool metadata, bool reported) {
    int ret = 0;
    char request[1024];
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;
    int timestamp = 0;
    int nonce = 0;

    timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);

    tc_iot_json_writer_open(w, request, sizeof(request));
    tc_iot_json_writer_string(w ,"method", "get");
    tc_iot_json_writer_bool(w ,"metadata", metadata);
    tc_iot_json_writer_bool(w ,"reported", reported);
    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    } else {
        tc_iot_hal_printf("[client->server]:%s\n", request);
    }

    nonce = tc_iot_hal_random();
    ret = tc_iot_http_mqapi_rpc( result, result_len, TC_IOT_CONFIG_RPC_API_URL, NULL, timestamp, nonce, p_device_info, request);
    if (ret > 0) {
        tc_iot_hal_printf("[server->client]:%s\n", result);
    } else {
        TC_IOT_LOG_INFO("request failed ,ret=%d", ret);
    }
    return ret;
}

int do_rpc_update(char * result, int result_len, tc_iot_device_info * p_device_info, 
         unsigned char * report_bits, tc_iot_shadow_local_data * p_local_data) {

    int ret = 0;
    char request[1024];
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;
    int timestamp = 0;
    int nonce = 0;

    timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);

    tc_iot_json_writer_open(w, request, sizeof(request));
    tc_iot_json_writer_string(w ,"method", "update");
    tc_iot_json_writer_object_begin(w ,"state");
    tc_iot_json_writer_object_begin(w ,"reported");
    if (TC_IOT_BIT_GET(report_bits, TC_IOT_PROP_param_bool)) {
        tc_iot_json_writer_raw_data(w ,"param_bool", p_local_data->param_bool ? TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE);
    }

    if (TC_IOT_BIT_GET(report_bits, TC_IOT_PROP_param_number)) {
        tc_iot_json_writer_int(w ,"param_number", p_local_data->param_number);
    }

    if (TC_IOT_BIT_GET(report_bits, TC_IOT_PROP_param_enum)) {
        tc_iot_json_writer_int(w ,"param_enum", p_local_data->param_enum);
    }

    if (TC_IOT_BIT_GET(report_bits, TC_IOT_PROP_param_string)) {
        tc_iot_json_writer_string(w ,"param_string", p_local_data->param_string);
    }

    tc_iot_json_writer_object_end(w);
    tc_iot_json_writer_object_end(w);
    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    } else {
        tc_iot_hal_printf("[c->s]:%s\n", request);
    }

    nonce = tc_iot_hal_random();
    ret = tc_iot_http_mqapi_rpc( result, result_len, TC_IOT_CONFIG_RPC_API_URL, NULL, timestamp, nonce, p_device_info, request);
    if (ret > 0) {
        tc_iot_hal_printf("[s->c]:%s\n", result);
    } else {
        TC_IOT_LOG_INFO("request failed ,ret=%d", ret);
    }
    return ret;
}

int do_rpc_delete(char * result, int result_len, tc_iot_device_info * p_device_info, 
         unsigned char * desired_bits) {

    int ret = 0;
    char request[1024];
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;
    int timestamp = 0;
    int nonce = 0;

    timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);

    tc_iot_json_writer_open(w, request, sizeof(request));
    tc_iot_json_writer_string(w ,"method", "delete");
    tc_iot_json_writer_object_begin(w ,"state");
    tc_iot_json_writer_object_begin(w ,"desired");
    if (TC_IOT_BIT_GET(desired_bits, TC_IOT_PROP_param_bool)) {
        tc_iot_json_writer_null(w ,"param_bool");
    }

    if (TC_IOT_BIT_GET(desired_bits, TC_IOT_PROP_param_number)) {
        tc_iot_json_writer_null(w ,"param_number");
    }

    if (TC_IOT_BIT_GET(desired_bits, TC_IOT_PROP_param_enum)) {
        tc_iot_json_writer_null(w ,"param_enum");
    }

    if (TC_IOT_BIT_GET(desired_bits, TC_IOT_PROP_param_string)) {
        tc_iot_json_writer_null(w ,"param_string");
    }

    tc_iot_json_writer_object_end(w);
    tc_iot_json_writer_object_end(w);
    ret = tc_iot_json_writer_close(w);
    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    } else {
        tc_iot_hal_printf("[client->server]:%s\n", request);
    }

    nonce = tc_iot_hal_random();
    ret = tc_iot_http_mqapi_rpc( result, result_len, TC_IOT_CONFIG_RPC_API_URL, NULL, timestamp, nonce, p_device_info, request);
    if (ret > 0) {
        tc_iot_hal_printf("[server->client]:%s\n", result);
    } else {
        TC_IOT_LOG_ERROR("request failed ,ret=%d", ret);
    }
    return ret;
}

static void step_log(const char * step_desc, bool begin) {
    if (begin) {
        tc_iot_hal_printf("\n-----%s--------%s---------------\n","begin", step_desc);
    } else {
        tc_iot_hal_printf("-----%s--------%s---------------\n"," end ", step_desc);
    }
}

int main(int argc, char** argv) {
    char result[1024];
    int ret = 0;
    unsigned char desired_bits[1];

    parse_command(&g_device_info, argc, argv);

    step_log("rpc get: server desired data", true);
    ret = do_rpc_get(result, sizeof(result), &g_device_info, false, false);
    if (ret < 0) {
        return 0;
    }
    step_log("rpc get: server desired data", false);

    memset(desired_bits, 0, sizeof(desired_bits));
    check_and_process_desired(desired_bits, &g_local_data, result);
    if (desired_bits[0]) {
        step_log("rpc update: report latest device state", true);
        ret = do_rpc_update(result, sizeof(result), &g_device_info, desired_bits, &g_local_data);
        if (ret < 0) {
            return 0;
        }
        step_log("rpc update: report latest device state", false);

        step_log("rpc delete: clear desired data after successfully processed.", true);
        ret = do_rpc_delete(result, sizeof(result), &g_device_info, desired_bits);
        if (ret < 0) {
            return 0;
        }
        step_log("rpc delete: clear desired data after successfully processed.", false);

        step_log("rpc get: fetch latest state", true);
        ret = do_rpc_get(result, sizeof(result), &g_device_info, false, true);
        if (ret < 0) {
            return 0;
        }
        step_log("rpc get: fetch latest state", false);

        tc_iot_hal_printf("\n> control command process finished, check latest report data over console:\n> https://console.cloud.tencent.com/iotsuite/product\n");
    } else {
        tc_iot_hal_printf("\n> no control command, try send control data over console:\n> https://console.cloud.tencent.com/iotsuite/product\n");
    }

    return 0;
}

