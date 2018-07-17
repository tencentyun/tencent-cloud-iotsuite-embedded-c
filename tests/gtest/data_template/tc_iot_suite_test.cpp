extern "C" {
#include "tc_iot_inc.h"
#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
}
#include "gtest/gtest.h"


extern tc_iot_shadow_config g_tc_iot_shadow_config;
extern tc_iot_shadow_local_data g_tc_iot_device_local_data;

void do_sim_data_change(void) {
    TC_IOT_LOG_TRACE("simulate data change.");
    int i = 0;

    g_tc_iot_device_local_data.param_bool = !g_tc_iot_device_local_data.param_bool;

    g_tc_iot_device_local_data.param_enum += 1;
    g_tc_iot_device_local_data.param_enum %= 3;

    g_tc_iot_device_local_data.param_number += 1;
    g_tc_iot_device_local_data.param_number = g_tc_iot_device_local_data.param_number > 4095?0:g_tc_iot_device_local_data.param_number;

    for (i = 0; i < 0+1;i++) {
        g_tc_iot_device_local_data.param_string[i] += 1;
        g_tc_iot_device_local_data.param_string[i] = g_tc_iot_device_local_data.param_string[0] > 'Z'?'A':g_tc_iot_device_local_data.param_string[0];
        g_tc_iot_device_local_data.param_string[i] = g_tc_iot_device_local_data.param_string[0] < 'A'?'A':g_tc_iot_device_local_data.param_string[0];
    }
    g_tc_iot_device_local_data.param_string[0+2] = 0;


    /* 上报数据最新状态 */
    tc_iot_report_device_data(tc_iot_get_shadow_client());
}

TEST(IOTSUITE, basic)
{
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();
    tc_iot_mqtt_client_config * p_client_config;

    const char * product_key =   getenv("TC_IOT_DATATEMPLATE_PRODUCT_KEY");
    const char * product_id =    getenv("TC_IOT_DATATEMPLATE_PRODUCT_ID");
    const char * device_name =   getenv("TC_IOT_DATATEMPLATE_DEVICE_NAME");
    const char * device_secret = getenv("TC_IOT_DATATEMPLATE_DEVICE_SECRET");

    if (!product_key || !product_id || !device_name || !device_secret) {
        std::cout << "MQTT test variable not found, please add settings to your .bashrc or .zshrc:" << std::endl;
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
    snprintf(g_tc_iot_shadow_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_tc_iot_shadow_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_PUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    ret = http_refresh_auth_token_with_expire(
                                              TC_IOT_CONFIG_AUTH_API_URL, TC_IOT_CONFIG_ROOT_CA,
                                              timestamp, nonce,
                                              &p_client_config->device_info,
                                              TC_IOT_TOKEN_MAX_EXPIRE_SECOND
                                              );

    ASSERT_EQ(ret, TC_IOT_SUCCESS);


    ret = tc_iot_server_init(tc_iot_get_shadow_client(), &g_tc_iot_shadow_config);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    ret = tc_iot_server_loop(tc_iot_get_shadow_client(), 2000);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    // while (!stop) {
    //     tc_iot_server_loop(tc_iot_get_shadow_client(), 200);
    //     for (i = 5; i > 0; i--) {
    //         tc_iot_hal_printf("%d ...\n", i);
    //         tc_iot_hal_sleep_ms(1000);
    //     }
    //     do_sim_data_change();
    // }

    tc_iot_server_destroy(tc_iot_get_shadow_client());
    return ;
}


