#include "tc_iot_device_config.h"
#include "tc_iot_export.h"

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

char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_SUB_TOPIC_DEF;
char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_PUB_TOPIC_DEF;

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;

    p_client_config = &(g_client_config);
    parse_command(p_client_config, argc, argv);

    use_static_token = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);
    _refresh_token();

    /* 用户自定义 Topic ，格式一般固定以 prodcut id 和 device name为前缀， */
    /* 格式为：{$product_id}/{$device_name}/xxx/yyy/zzz .... */
    /* 具体后缀根据实际业务情况来定义，自行创建。 */
    const char * custom_topic = "%s/%s/user/get";
    snprintf(sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, custom_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN,  custom_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    tc_iot_hal_printf("sub topic: %s\n", sub_topic);
    tc_iot_hal_printf("pub topic: %s\n", pub_topic);
    run_mqtt(&g_client_config);
}

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("[s->c] %.*s\n", (int)message->payloadlen, (char*)message->payload);
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
    tc_iot_hal_printf("UNHANDLED [s->c] %.*s\n", (int)message->payloadlen, (char*)message->payload);
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
    int timeout = 2000;

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
                                           _on_message_received, NULL);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("subscribe topic %s failed, trouble shooting guide: " "%s#%d\n", sub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }

    tc_iot_mqtt_client_yield(p_client, timeout);

    while (!stop) {
        /* 具体消息格式可自行定义，注意保持为 JSON 格式*/
        action_get = "{\"method\":\"notify\", \"device_status\":{\"light\":\"on\",\"level\":8}}";

        tc_iot_mqtt_message pubmsg;
        memset(&pubmsg, '\0', sizeof(pubmsg));
        pubmsg.payload = action_get;
        pubmsg.payloadlen = strlen(pubmsg.payload);
        pubmsg.qos = TC_IOT_QOS1;
        pubmsg.retained = 0;
        pubmsg.dup = 0;
        tc_iot_hal_printf("[c->s] publishing msg\n");
        ret = tc_iot_mqtt_client_publish(p_client, pub_topic, &pubmsg);
        if (TC_IOT_SUCCESS != ret) {
            if (ret != TC_IOT_MQTT_RECONNECT_IN_PROGRESS) {
                tc_iot_hal_printf("publish to topic %s failed, trouble shooting guide: " "%s#%d\n", pub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
                break;
            } else {
                tc_iot_hal_printf("publish to topic %s failed, try reconnecting, "
                        "or visit trouble shooting guide: " "%s#%d\n", pub_topic, TC_IOT_TROUBLE_SHOOTING_URL, ret);
            }
        }

        tc_iot_mqtt_client_yield(p_client, timeout);
    }

    tc_iot_mqtt_client_disconnect(p_client);
    return 0;
}

