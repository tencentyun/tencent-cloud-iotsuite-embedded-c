#include "tc_iot_device_config.h"
#include "tc_iot_export.h"

static volatile int stop = 0;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop = 1;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop = 1;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
}

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    /* tc_iot_hal_printf("->%.*s\t", md->topicName->lenstring.len, */
           /* md->topicName->lenstring.data); */
    tc_iot_hal_printf("[s->c] %.*s\n", (int)message->payloadlen, (char*)message->payload);
}

tc_iot_shadow_config g_client_config = {
    {
        {
            // device info
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
            TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
        },
        TC_IOT_CONFIG_SERVER_HOST,
        TC_IOT_CONFIG_SERVER_PORT,
        TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
        TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
        TC_IOT_CONFIG_CLEAN_SESSION,
        TC_IOT_CONFIG_USE_TLS,
        TC_IOT_CONFIG_AUTO_RECONNECT,
        TC_IOT_CONFIG_ROOT_CA,
        TC_IOT_CONFIG_CLIENT_CRT,
        TC_IOT_CONFIG_CLIENT_KEY,
        NULL,
        NULL,
    },
    SUB_TOPIC,
    PUB_TOPIC,
    _on_message_received,
};

int main(int argc, char** argv) {
    tc_iot_shadow_client client;
    tc_iot_shadow_client* p_shadow_client = &client;
    int ret = 0;

    tc_iot_hal_printf("requesting username and password for mqtt.\n");
    ret = http_refresh_auth_token(
        TC_IOT_CONFIG_AUTH_API_URL, NULL,
        &g_client_config.mqtt_client_config.device_info);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("refresh token failed, visit: https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/wiki/trouble_shooting#%d\n.", ret);
        return 0;
    }
    tc_iot_hal_printf("request username and password for mqtt success.\n");

    tc_iot_hal_printf("constructing mqtt shadow client.\n");
    ret = tc_iot_shadow_construct(p_shadow_client, &g_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("construct shadow failed, visit: https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/wiki/trouble_shooting#%d\n.", ret);
        return 0;
    }

    tc_iot_hal_printf("construct mqtt shadow client success.\n");
    int timeout = TC_IOT_CONFIG_COMMAND_TIMEOUT_MS;
    tc_iot_hal_printf("yield waiting for server push.\n");
    tc_iot_shadow_yield(p_shadow_client, timeout);
    tc_iot_hal_printf("yield waiting for server finished.\n");

    tc_iot_hal_printf("[c->s] shadow_get\n");
    tc_iot_shadow_get(p_shadow_client);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    char* action_update_all =
        "{\"method\":\"update\",\"state\":{\"reported\":{\"temperature\":1023,"
        "\"switch\":1023},\"desired\":{\"temperature\":1024,\"switch\":1024}}}";
    tc_iot_hal_printf("[c->s] shadow_update_all\n");
    tc_iot_shadow_update(p_shadow_client, action_update_all);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    char* action_update_reported =
        "{\"method\":\"update\",\"state\":{\"reported\":{\"temperature\":1023,"
        "\"switch\":1023}}}";
    tc_iot_hal_printf("[c->s] shadow_update_reported\n");
    tc_iot_shadow_update(p_shadow_client, action_update_reported);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    char* action_update_desired =
        "{\"method\":\"update\",\"state\":{\"desired\":{\"temperature\":1024,\"switch\":1024}}}";
    tc_iot_hal_printf("[c->s] shadow_update_desired\n");
    tc_iot_shadow_update(p_shadow_client, action_update_desired);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    char* action_delete =
        "{\"method\":\"delete\",\"state\":{\"reported\":{\"temperature\":null},"
        "\"desired\":null}}";
    tc_iot_hal_printf("[c->s] shadow_delete\n");
    tc_iot_shadow_delete(p_shadow_client, action_delete);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    tc_iot_hal_printf("Stopping\n");
    tc_iot_shadow_destroy(p_shadow_client);
    tc_iot_hal_printf("Exit success.\n");
    return 0;
}
