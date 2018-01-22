#include "tc_iot_device_config.h"
#include "tc_iot_export.h"
int run_simple_mqtt_client();

tc_iot_mqtt_client_config g_client_config = {
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
};

int main(int argc, char** argv) {
    int ret = 0;
    printf("requesting username and password for mqtt.\n");
    ret = http_refresh_auth_token(
        TC_IOT_CONFIG_AUTH_API_URL, NULL,
        &g_client_config.device_info);
    if (ret != TC_IOT_SUCCESS) {
        printf("refresh token failed, visit: https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/wiki/trouble_shooting#%d\n.", ret);
        return 0;
    }
    printf("request username and password for mqtt success.\n");

    // 连接mqtt服务器
    run_simple_mqtt_client(&g_client_config);
}
