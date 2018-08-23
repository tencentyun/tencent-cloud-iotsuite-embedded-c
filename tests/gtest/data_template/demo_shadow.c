#include "tc_iot_device_config.h"
#include "tc_iot_export.h"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

extern void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv);
int run_shadow(tc_iot_shadow_config * p_client_config);

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

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("[s->c] %s\n", (char*)message->payload);
}
void _message_ack_callback(tc_iot_command_ack_status_e ack_status,
        tc_iot_message_data * md , void * p_context) {

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            tc_iot_hal_printf("request timeout\n");
        }
        return;
    }

    tc_iot_mqtt_message* message = md->message;

    tc_iot_hal_printf("[s->c] %s\n", (char*)message->payload);
}

tc_iot_shadow_config g_client_config = {
    {
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
        NULL,
        NULL,
    },
    TC_IOT_SUB_TOPIC_DEF,
    TC_IOT_PUB_TOPIC_DEF,
    _on_message_received,
};

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;
    bool use_static_token;
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    setbuf(stdout, NULL);

    p_client_config = &(g_client_config.mqtt_client_config);
    parse_command(p_client_config, argc, argv);
    snprintf(g_client_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_client_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_PUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    use_static_token = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);

    if (!use_static_token) {
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = http_refresh_auth_token_with_expire(
                TC_IOT_CONFIG_AUTH_API_URL, TC_IOT_CONFIG_ROOT_CA,
                timestamp, nonce,
                &p_client_config->device_info,
                TC_IOT_TOKEN_MAX_EXPIRE_SECOND
                );
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return 0;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }

    run_shadow(&g_client_config);

    return 0;
}

int run_shadow(tc_iot_shadow_config * p_client_config) {
    int timeout;
    int ret = 0;
    char buffer[512];
    int buffer_len = sizeof(buffer);
    char reported[256];
    char desired[256];
    tc_iot_shadow_client client;
    tc_iot_shadow_client* p_shadow_client = &client;

    tc_iot_hal_printf("constructing mqtt shadow client.\n");
    ret = tc_iot_shadow_construct(p_shadow_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("construct shadow client failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return 0;
    }

    tc_iot_hal_printf("construct mqtt shadow client success.\n");
    timeout = 2000;
    tc_iot_hal_printf("yield waiting for server push.\n");
    tc_iot_shadow_yield(p_shadow_client, timeout);
    tc_iot_hal_printf("yield waiting for server finished.\n");

    tc_iot_shadow_get(p_shadow_client, buffer, buffer_len, _message_ack_callback, 6000, NULL);
    tc_iot_hal_printf("[c->s] shadow_get\n%s\n", buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    snprintf(reported, sizeof(reported), "{\"param_number\":%d,\"param_enum\":%d,\"param_bool\":%s}", 12345,1, TC_IOT_SHADOW_JSON_TRUE);

    tc_iot_shadow_update(p_shadow_client, buffer, buffer_len, reported, NULL, _message_ack_callback, 6000, NULL);
    tc_iot_hal_printf("[c->s] shadow_update_reported\n%s\n", buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    snprintf(desired, sizeof(desired),"{\"param_number\":null,\"param_enum\":null,\"param_bool\":null}");
    tc_iot_shadow_delete(p_shadow_client, buffer, buffer_len, NULL, desired, _message_ack_callback, 6000, NULL);
    tc_iot_hal_printf("[c->s] shadow_clear_desired\n%s\n", buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    tc_iot_shadow_delete(p_shadow_client, buffer, buffer_len, NULL, TC_IOT_JSON_NULL, _message_ack_callback, 6000, NULL);
    tc_iot_hal_printf("[c->s] shadow_clear_desired\n%s\n", buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    tc_iot_hal_printf("Stopping\n");
    tc_iot_shadow_destroy(p_shadow_client);
    tc_iot_hal_printf("Exit success.\n");
    return 0;
}
