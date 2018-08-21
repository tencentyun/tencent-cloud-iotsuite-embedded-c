extern "C" {
#include "tc_iot_export.h"
#include "tc_iot_device_config.h"
}

#include "tc_iot_device_logic.h"
#include "gtest/gtest.h"

extern void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv);
int run_shadow(tc_iot_shadow_config * p_client_config);

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

tc_iot_shadow_config g_tc_iot_shadow_config = {
    {
        {
            /* device info*/
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
            TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
            TC_IOT_CONFIG_AUTH_MODE, TC_IOT_CONFIG_REGION, TC_IOT_CONFIG_AUTH_API_URL,
        },
        (char *)TC_IOT_CONFIG_MQ_SERVER_HOST,
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
        NULL,
        NULL,
        0,  /* send will */
        {
            {'M', 'Q', 'T', 'W'}, 0, {NULL, {0, NULL}}, {NULL, {0, NULL}}, 0, 0,
        }
    },
    TC_IOT_SHADOW_SUB_TOPIC_DEF,
    TC_IOT_SHADOW_PUB_TOPIC_DEF,
    _on_message_received,
};

TEST(IOTSUITE, data_template)
{
    tc_iot_mqtt_client_config * p_client_config;
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();
    int timeout;
    char buffer[512];
    int buffer_len = sizeof(buffer);
    char reported[256];
    char desired[256];
    tc_iot_shadow_client client;
    tc_iot_shadow_client* p_shadow_client = &client;

    const char * product_key =   getenv("TC_IOT_DATATEMPLATE_PRODUCT_KEY");
    const char * product_id =    getenv("TC_IOT_DATATEMPLATE_PRODUCT_ID");
    const char * device_name =   getenv("TC_IOT_DATATEMPLATE_DEVICE_NAME");
    const char * device_secret = getenv("TC_IOT_DATATEMPLATE_DEVICE_SECRET");

    if (!product_key || !product_id || !device_name || !device_secret) {
        std::cout << "DataTemplate test variable not found, please add settings to your .bashrc or .zshrc:" << std::endl;
        std::cout << 
            "export TC_IOT_DATATEMPLATE_PRODUCT_ID=\"Your-PRODUCT-ID\"\n"
            "export TC_IOT_DATATEMPLATE_PRODUCT_KEY=\"Your-PRODUCT-KEY\"\n"
            "export TC_IOT_DATATEMPLATE_DEVICE_NAME=\"Your-DEVICE-NAME\"\n"
            "export TC_IOT_DATATEMPLATE_DEVICE_SECRET=\"Your-DEVICE-SECRET\"\n";
    }

    ASSERT_STRNE(product_key,NULL);
    ASSERT_STRNE(product_id,NULL);
    ASSERT_STRNE(device_name,NULL);
    ASSERT_STRNE(device_secret,NULL);

    p_client_config = &(g_tc_iot_shadow_config.mqtt_client_config);

    strcpy(p_client_config->device_info.product_id, product_id);
    strcpy(p_client_config->device_info.device_name, device_name);
    strcpy(p_client_config->device_info.secret, device_secret);
    sprintf(p_client_config->device_info.client_id, "%s@%s", product_key, device_name);

    /* 根据 product id 和device name 定义，生成发布和订阅的 Topic 名称。 */
    snprintf(g_tc_iot_shadow_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SHADOW_SUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_tc_iot_shadow_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SHADOW_PUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    ret = tc_iot_refresh_auth_token( timestamp, nonce, &p_client_config->device_info, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);

    ASSERT_EQ(ret, TC_IOT_SUCCESS);



    tc_iot_hal_printf("constructing mqtt shadow client.\n");
    tc_iot_shadow_construct(p_shadow_client, &g_tc_iot_shadow_config);

    tc_iot_hal_printf("construct mqtt shadow client success.\n");
    timeout = 200;
    tc_iot_hal_printf("yield waiting for server push.\n");

    tc_iot_shadow_yield(p_shadow_client, timeout);
    tc_iot_hal_printf("yield waiting for server finished.\n");

    ret = tc_iot_shadow_get(p_shadow_client, buffer, buffer_len, _message_ack_callback, 2000, NULL);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    tc_iot_hal_printf("[c->s] shadow_get\n%s\n", buffer);
    ret = tc_iot_shadow_yield(p_shadow_client, timeout);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);
    

    snprintf(reported, sizeof(reported), "{\"param_number\":%d,\"param_enum\":%d,\"param_bool\":%s}", 12345,1, TC_IOT_SHADOW_JSON_TRUE);

    ret = tc_iot_shadow_update(p_shadow_client, buffer, buffer_len, reported, NULL, _message_ack_callback, 2000, NULL);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    tc_iot_hal_printf("[c->s] shadow_update_reported\n%s\n", buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    snprintf(desired, sizeof(desired),"{\"param_number\":null,\"param_enum\":null,\"param_bool\":null}");
    ret = tc_iot_shadow_delete(p_shadow_client, buffer, buffer_len, NULL, desired, _message_ack_callback, 2000, NULL);

    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    tc_iot_hal_printf("[c->s] shadow_clear_desired\n%s\n", buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    ret = tc_iot_shadow_delete(p_shadow_client, buffer, buffer_len, NULL, TC_IOT_JSON_NULL, _message_ack_callback, 2000, NULL);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    tc_iot_hal_printf("[c->s] shadow_clear_desired\n%s\n", buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    while (tc_iot_shadow_pending_session_count(p_shadow_client) > 0) {
        tc_iot_shadow_yield(p_shadow_client, timeout);
    }

    tc_iot_hal_printf("Stopping\n");
    tc_iot_shadow_destroy(p_shadow_client);
    tc_iot_hal_printf("Exit success.\n");
    return ;
}
