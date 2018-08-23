#include "tc_iot_device_config.h"
#include <sys/stat.h>
#include "tc_iot_ota_logic.h"
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

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;

    if (argc == 2 && strcmp(argv[1],"--firm-version") == 0) {
        tc_iot_hal_printf("%s\n", TC_IOT_FIRM_VERSION);
        return 0;
    }

    p_client_config = &(g_client_config);
    parse_command(p_client_config, argc, argv);

    use_static_token = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);
    _refresh_token();

    run_mqtt(&g_client_config);
}

void _tc_iot_on_mqtt_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    TC_IOT_LOG_TRACE("[s->c] %s\n", (char*)message->payload);
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
    tc_iot_ota_handler * ota_handler = &handler;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    tc_iot_mqtt_client client;
    tc_iot_mqtt_client* p_client = &client;

    // 初始化 MQTT Client 用于发送和订阅 OTA 消息。
    ret = tc_iot_mqtt_client_construct(p_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("connect MQTT server failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return TC_IOT_FAILURE;
    }

    // 订阅 OTA 下行 Topic，格式为： /ota/get/${product_id}/${device_name}
    tc_iot_hal_snprintf(ota_sub_topic, sizeof(ota_sub_topic), "ota/get/%s/%s", 
            p_client_config->device_info.product_id, 
            p_client_config->device_info.device_name);

    // 配置 OTA 上行 Topic，格式为： /ota/update/${product_id}/${device_name}
    tc_iot_hal_snprintf(ota_pub_topic, sizeof(ota_pub_topic), "ota/update/%s/%s", 
            p_client_config->device_info.product_id, 
            p_client_config->device_info.device_name);

    // 初始化 OTA 服务
    ret = tc_iot_ota_construct(ota_handler, p_client, ota_sub_topic, ota_pub_topic, _on_ota_message_received);

    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("init ota handler failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
    }

    // 上报设备信息及当前版本号
    tc_iot_ota_report_firm(&handler,
            "sdk-ver", TC_IOT_SDK_VERSION,  // 上报 SDK 版本
            "firm-ver",TC_IOT_FIRM_VERSION,  // 上报固件信息，OTA 升级版本号判断依据
            NULL); // 最后一个参数固定填写 NULL，作为变参结束判断

    while (!stop) {
        // 设备业务主循环
        tc_iot_mqtt_client_yield(p_client, timeout);
    }
    
    tc_iot_ota_destroy(&handler);

    tc_iot_mqtt_client_disconnect(p_client);
    tc_iot_mqtt_client_destroy(p_client);
    return 0;
}

