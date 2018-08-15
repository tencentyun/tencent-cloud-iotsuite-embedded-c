extern "C" {
#include "tc_iot_export.h"
#include "tc_iot_device_config.h"
}

#include "tc_iot_device_logic.h"
#include "gtest/gtest.h"


extern tc_iot_shadow_config g_tc_iot_shadow_config;
extern tc_iot_shadow_local_data g_tc_iot_device_local_data;
void operate_device(tc_iot_shadow_local_data * device) {
    // TC_IOT_LOG_TRACE("checking control status");
}


TEST(IOTSUITE, data_template)
{
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();
    tc_iot_mqtt_client_config * p_client_config;
    char buffer[512];
    int buffer_len = sizeof(buffer);
    char desired[256];

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


    ret = tc_iot_server_init(tc_iot_get_shadow_client(), &g_tc_iot_shadow_config);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    while (tc_iot_shadow_pending_session_count(tc_iot_get_shadow_client()) > 0) {
        tc_iot_server_loop(tc_iot_get_shadow_client(), 200);
    }

    snprintf(desired, sizeof(desired),"{\"param_bool\":true,\"param_enum\":1,\"param_number\":2,\"param_string\":\"test\"}");
    tc_iot_shadow_update(tc_iot_get_shadow_client(), buffer, buffer_len, NULL, desired, NULL, 0, NULL);

    ret = tc_iot_server_loop(tc_iot_get_shadow_client(), 2000);

    ASSERT_EQ(g_tc_iot_device_local_data.param_bool, true);
    ASSERT_EQ(g_tc_iot_device_local_data.param_enum, 1);
    ASSERT_EQ(g_tc_iot_device_local_data.param_number, 2);
    ASSERT_STREQ(g_tc_iot_device_local_data.param_string, "test");

    tc_iot_server_destroy(tc_iot_get_shadow_client());
    return ;
}


