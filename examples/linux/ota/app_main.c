#include "../tc_iot_device_config.h"
#include "tc_iot_export.h"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"
#define TC_IOT_MAX_FIELD_LEN  22

extern void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv);

bool use_static_token = false;

void my_default_msg_handler(tc_iot_message_data*);
void my_disconnect_handler(tc_iot_mqtt_client* c, void* ctx);
void _refresh_token();

int run_mqtt(tc_iot_mqtt_client_config* p_client_config);

tc_iot_mqtt_client_config g_client_config = {
    {
        /* device info*/
        TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
        TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
        TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
    },
    TC_IOT_CONFIG_SERVER_HOST,
    TC_IOT_CONFIG_SERVER_PORT,
    TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
    TC_IOT_CONFIG_TLS_HANDSHAKE_TIMEOUT_MS,
    TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
    TC_IOT_CONFIG_CLEAN_SESSION,
    TC_IOT_CONFIG_USE_TLS,
    TC_IOT_CONFIG_AUTO_RECONNECT,
    TC_IOT_CONFIG_ROOT_CA,
    TC_IOT_CONFIG_CLIENT_CRT,
    TC_IOT_CONFIG_CLIENT_KEY,
    my_disconnect_handler,
    my_default_msg_handler,
};

tc_iot_ota_handler handler;

char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_SUB_TOPIC_DEF;
char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_PUB_TOPIC_DEF;

int main(int argc, char** argv) {
    int ret = 0;
    tc_iot_mqtt_client_config * p_client_config;

    p_client_config = &(g_client_config);
    parse_command(p_client_config, argc, argv);

    use_static_token = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);
    _refresh_token();

    /* 用户自定义 Topic ，格式一般固定以 prodcut id 和 device name为前缀， */
    /* 格式为：{$product_id}/{$device_name}/xxx/yyy/zzz .... */
    /* 具体后缀根据实际业务情况来定义，自行创建。 
     * 本demo运行需要创建的Topic为： {$product_id}/{$device_name}/user/get
     * */
    const char * custom_topic = "%s/%s/user/get";
    snprintf(sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, custom_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN,  custom_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    tc_iot_hal_printf("sub topic: %s\n", sub_topic);
    tc_iot_hal_printf("pub topic: %s\n", pub_topic);
    run_mqtt(&g_client_config);
}

void _on_ota_message_received(tc_iot_message_data* md) {
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    char field_buf[TC_IOT_MAX_FIELD_LEN];
    int field_index = 0;
    int ret = 0;
    tc_iot_mqtt_client * p_mqtt_client = (tc_iot_mqtt_client *)md->mqtt_client;
    tc_iot_mqtt_message* message = md->message;

    TC_IOT_LOG_TRACE("[s->c] %s", (char*)message->payload);
    memset(field_buf, 0, sizeof(field_buf));

    /* 有效性检查 */
    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        return ;
    }

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "method", field_buf, sizeof(field_buf));
    if (field_index <= 0 ) {
        TC_IOT_LOG_ERROR("field method not found in JSON: %s", (char*)message->payload);
        return ;
    }

    if (strncmp(TC_IOT_MQTT_METHOD_CONTROL, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("Control data receved.");
    } else if (strncmp(TC_IOT_MQTT_METHOD_REPLY, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("Reply pack recevied.");
    } else if (strncmp(TC_IOT_MQTT_METHOD_REPORT_FIRM, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("request report firm info.");
        tc_iot_ota_report_firm(&handler,
                "product", g_client_config.device_info.product_id,
                "device", g_client_config.device_info.device_name, "sdk-ver", TC_IOT_SDK_VERSION,
                "firm-ver","Linux_V1.00", NULL);
        return;
    }
}

void _refresh_token() {
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();
    tc_iot_mqtt_client_config * p_client_config;

    p_client_config = &(g_client_config);

    if (!use_static_token) {
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = http_refresh_auth_token(
                TC_IOT_CONFIG_AUTH_API_URL, TC_IOT_CONFIG_ROOT_CA,
                timestamp, nonce,
                &p_client_config->device_info);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }
}

void my_disconnect_handler(tc_iot_mqtt_client* c, void* ctx) {
    /* 自动刷新 password*/
    if (!use_static_token && tc_iot_mqtt_get_auto_reconnect(c)) {
        /* _refresh_token(); */
    }
}

void my_default_msg_handler(tc_iot_message_data * md) {
    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("UNHANDLED [s->c] %s\n", (char*)message->payload);
}


static volatile int stop = 0;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop ++;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop ++;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
    if (stop >= 3) {
        tc_iot_hal_printf("SIGINT/SIGTERM received over %d times, force shutdown now.\n", stop);
        exit(0);
    }
}


int run_mqtt(tc_iot_mqtt_client_config* p_client_config) {
    int ret;
    char * action_get;
    int timeout = 200;

    tc_iot_ota_handler * ota_handler = &handler;


    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    tc_iot_mqtt_client client;
    tc_iot_mqtt_client* p_client = &client;
    ret = tc_iot_mqtt_client_construct(p_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("connect MQTT server failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }
    ret = tc_iot_mqtt_client_subscribe(p_client, sub_topic, TC_IOT_QOS1,
                                           _on_ota_message_received, NULL);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("subscribe topic %s failed, trouble shooting guide: " "%s#%d\n", sub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }

    tc_iot_mqtt_client_yield(p_client, timeout);

    ret = tc_iot_ota_init(ota_handler, p_client, 
            p_client_config->device_info.product_id, 
            p_client_config->device_info.device_name);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("init ota handler failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
    }
    tc_iot_ota_set_ota_id(ota_handler, "ota_100000");

    tc_iot_ota_report_firm(&handler,
            "product", g_client_config.device_info.product_id,
            "device", g_client_config.device_info.device_name, "sdk-ver", TC_IOT_SDK_VERSION,
            "firm-ver","Linux_V1.00", NULL);

    tc_iot_ota_report(ota_handler, OTA_COMMAND_RECEIVED, NULL, 0);
    tc_iot_ota_report(ota_handler, OTA_DOWNLOAD, NULL, 0);
    tc_iot_ota_report(ota_handler, OTA_DOWNLOAD, NULL, 100);
    tc_iot_ota_report(ota_handler, OTA_MD5_CHECK, NULL, 0);
    tc_iot_ota_report(ota_handler, OTA_START_UPGRADE, NULL, 0);
    tc_iot_ota_report(ota_handler, OTA_UPGRADING, NULL, 0);

    while (!stop) {
        tc_iot_mqtt_client_yield(p_client, timeout);
    }

    tc_iot_mqtt_client_disconnect(p_client);
}

