extern "C" {
#include "tc_iot_export.h"
#include "tc_iot_device_config.h"
}
#include "gtest/gtest.h"

void my_default_msg_handler(tc_iot_message_data*);
void my_default_msg_handler(tc_iot_message_data * md) {
  tc_iot_mqtt_message* message = md->message;
  tc_iot_hal_printf("UNHANDLED [s->c] %s\n", (char*)message->payload);
}

void _on_message_received(tc_iot_message_data* md) {
    char * _resp_data = (char *)md->context;
    tc_iot_mqtt_message* message = md->message;
    strcpy(_resp_data, (char *)message->payload);
    TC_IOT_LOG_TRACE("[s->c]:%s", _resp_data);
}

TEST(MQTT, custom_topic)
{
    char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_SHADOW_SUB_TOPIC_DEF;
    char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_SHADOW_PUB_TOPIC_DEF;

    static char _req_data[1024];
    static char _resp_data[1024];
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();
    tc_iot_mqtt_client_config * p_client_config;
    int delay_ms = 5000;
    int delay_interval = 200;
    tc_iot_mqtt_client client;
    tc_iot_mqtt_client* p_client = &client;
    tc_iot_mqtt_client_config _client_config = {
        {
            /* device info*/
            "device_secret", "iot-product-id",
            "device_name", "product_key@device_name",
            "", "", 0,
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
        my_default_msg_handler,
    };
    const char * custom_topic = "%s/%s/cmd";
    const char * product_key = getenv("TC_IOT_MQTT_PRODUCT_KEY");
    const char * product_id = getenv("TC_IOT_MQTT_PRODUCT_ID");
    const char * device_name = getenv("TC_IOT_MQTT_DEVICE_NAME");
    const char * device_secret = getenv("TC_IOT_MQTT_DEVICE_SECRET");


    if (!product_key || !product_id || !device_name || !device_secret) {
        std::cout << "MQTT test variable not found, please add settings to your .bashrc or .zshrc:" << std::endl;
        std::cout << 
        "export TC_IOT_MQTT_PRODUCT_ID=\"Your-PRODUCT-ID\"\n"
        "export TC_IOT_MQTT_PRODUCT_KEY=\"Your-PRODUCT-KEY\"\n"
        "export TC_IOT_MQTT_DEVICE_NAME=\"Your-DEVICE-NAME\"\n"
        "export TC_IOT_MQTT_DEVICE_SECRET=\"Your-DEVICE-SECRET\"\n";
    }
    ASSERT_STRNE(product_key,NULL);
    ASSERT_STRNE(product_id,NULL);
    ASSERT_STRNE(device_name,NULL);
    ASSERT_STRNE(device_secret,NULL);

    p_client_config = &(_client_config);

    strcpy(p_client_config->device_info.product_id, product_id);
    strcpy(p_client_config->device_info.device_name, device_name);
    strcpy(p_client_config->device_info.secret, device_secret);
    sprintf(p_client_config->device_info.client_id, "%s@%s", product_key, device_name);


    snprintf(sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, custom_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN,  custom_topic, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    ret = tc_iot_refresh_auth_token( timestamp, nonce, &p_client_config->device_info, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);

    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    ret = tc_iot_mqtt_client_construct(p_client, p_client_config);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);
    ret = tc_iot_mqtt_client_subscribe(p_client, sub_topic, TC_IOT_QOS1,
                                           _on_message_received, _resp_data);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    tc_iot_mqtt_message pubmsg;

    memset(&pubmsg, '\0', sizeof(pubmsg));
    memset(_resp_data, 0, sizeof(_resp_data));

    sprintf(_req_data, "{\"method\":\"get\", \"passthrough\":{\"timestamp\":%ld}}", time(NULL));
    TC_IOT_LOG_TRACE("[c->s]:%s", _req_data);
    pubmsg.payload = _req_data;
    pubmsg.payloadlen = strlen(_req_data);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    ret = tc_iot_mqtt_client_publish(p_client, pub_topic, &pubmsg);
    ASSERT_GE(ret, TC_IOT_SUCCESS);

    while ((strlen(_resp_data) <= 0) && (delay_ms > 0)) {
        ret = tc_iot_mqtt_client_yield(p_client, delay_interval);
        delay_ms -= delay_interval;
    }
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    ASSERT_EQ(0, strcmp(_req_data, _resp_data));

    ret = tc_iot_mqtt_client_disconnect(p_client);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);
    tc_iot_mqtt_client_destroy(p_client);
    return ;
}


