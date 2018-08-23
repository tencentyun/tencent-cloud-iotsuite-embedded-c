#include "tc_iot_device_config.h"
#include "tc_iot_export.h"

typedef unsigned long timestamp_t;
typedef struct _smartbox_data {
    bool door_switch;
    char status[6];
    char fault[20];
} smartbox_data;

smartbox_data g_tc_smartbox_data = {
    true,
    "full",
    "no",
};


#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

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
        TC_IOT_CONFIG_AUTH_MODE, TC_IOT_CONFIG_REGION, TC_IOT_CONFIG_AUTH_API_URL,
    },
    TC_IOT_CONFIG_MQ_SERVER_HOST,
    TC_IOT_CONFIG_MQ_SERVER_PORT,
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

char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME "/cmd";
char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME "/update";

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;

    p_client_config = &(g_client_config);
    parse_command(p_client_config, argc, argv);

    use_static_token = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);
    _refresh_token();

    const char * update_topic = "%s/%s/update";
    const char * cmd_topic = "%s/%s/cmd";
    snprintf(sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, cmd_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN,  update_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    tc_iot_hal_printf("sub topic: %s\n", sub_topic);
    tc_iot_hal_printf("pub topic: %s\n", pub_topic);
    run_mqtt(&g_client_config);
}

void report_status(tc_iot_mqtt_client * p_client)
{
    int ret = 0;
    char action_report[256];
    /* 具体消息格式可自行定义，注意保持为 JSON 格式*/
    tc_iot_hal_snprintf(action_report, sizeof(action_report),
            "{\"door_switch\":%s,\"status\":\"%s\",\"fault\":\"%s\"}",
            g_tc_smartbox_data.door_switch?"true":"false",
            g_tc_smartbox_data.status,
            g_tc_smartbox_data.fault);

    tc_iot_mqtt_message pubmsg;
    memset(&pubmsg, '\0', sizeof(pubmsg));
    pubmsg.payload = action_report;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    tc_iot_hal_printf("[c->s] publishing msg\n%s:%s\n",pub_topic, action_report);
    ret = tc_iot_mqtt_client_publish(p_client, pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != ret) {
        if (ret != TC_IOT_MQTT_RECONNECT_IN_PROGRESS) {
            tc_iot_hal_printf("publish to topic %s failed, trouble shooting guide: " "%s#%d\n", 
                    pub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        } else {
            tc_iot_hal_printf("publish to topic %s failed, try reconnecting, "
                    "or visit trouble shooting guide: " "%s#%d\n", pub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        }
    }
}

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    char field_buf[32];
    int field_index = 0;
    int ret = 0;
    char * start = NULL;
    char * payload = NULL;
    /* int len = 0; */
    tc_iot_mqtt_client* p_client = NULL;

    tc_iot_hal_printf("[s->c] %s\n",  (char*)message->payload);

    p_client = md->mqtt_client;
    memset(field_buf, 0, sizeof(field_buf));

    /* 有效性检查 */
    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("parse payload failed:ret=%d", ret)
        return ;
    }

    payload = (char*)message->payload;
    field_index = tc_iot_json_find_token(payload, json_token, ret, "door_switch", field_buf, sizeof(field_buf));
    if (field_index > 0 ) {
        start = payload + json_token[field_index].start;
        /* len = json_token[field_index].end - json_token[field_index].start; */
        if (start[0] == 't') {
            g_tc_smartbox_data.door_switch = true;
        } else {
            if (!g_tc_smartbox_data.door_switch) {
                tc_iot_hal_snprintf(g_tc_smartbox_data.fault, sizeof(g_tc_smartbox_data.fault), "bad state");
            } else {
                g_tc_smartbox_data.door_switch = false;
                tc_iot_hal_snprintf(g_tc_smartbox_data.status, sizeof(g_tc_smartbox_data.status), "free");
            }
        }
    }

    report_status(p_client);
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
        ret = TC_IOT_AUTH_FUNC( timestamp, nonce, &p_client_config->device_info, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);
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
    int timeout = 200;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    setbuf(stdout, NULL);

    tc_iot_mqtt_client client;
    tc_iot_mqtt_client* p_client = &client;
    ret = tc_iot_mqtt_client_construct(p_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("connect MQTT server failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }
    ret = tc_iot_mqtt_client_subscribe(p_client, sub_topic, TC_IOT_QOS1,
                                           _on_message_received, NULL);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("subscribe topic %s failed, trouble shooting guide: " "%s#%d\n", 
                sub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }

    tc_iot_mqtt_client_yield(p_client, timeout);
    report_status(p_client);

    while (!stop) {
        tc_iot_mqtt_client_yield(p_client, timeout);
    }

    tc_iot_mqtt_client_disconnect(p_client);
    return 0;
}

