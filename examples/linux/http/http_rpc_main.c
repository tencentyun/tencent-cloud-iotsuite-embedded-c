#include "tc_iot_export.h"
#include "tc_iot_http_config.h"

extern void parse_command(tc_iot_device_info * p_device, int argc, char ** argv);

tc_iot_device_info g_device_info = {
    /* device info*/
    TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
    TC_IOT_CONFIG_DEVICE_NAME, "",
};

int main(int argc, char** argv) {
       
    char buffer[1024];
    int timestamp = time(NULL);
    int nonce = 0;
    int ret = 0;

    const char * message = "{\"method\":\"get\"}";

    tc_iot_hal_srandom(timestamp);
    nonce = tc_iot_hal_random();

    parse_command(&g_device_info, argc, argv);

    ret = http_mqapi_rpc(TC_IOT_CONFIG_RPC_API_URL, NULL, timestamp, nonce, &g_device_info, message, buffer,sizeof(buffer));
    if (ret > 0) {
        TC_IOT_LOG_INFO("Request success, result:\n%s", buffer);
    } else {
        TC_IOT_LOG_INFO("request failed ,ret=%d", ret);
    }

    message = "{\"method\":\"get\",\"meta\":false,\"report\":false}";

    ret = http_mqapi_rpc(TC_IOT_CONFIG_RPC_API_URL, NULL, timestamp, nonce, &g_device_info, message, buffer,sizeof(buffer));
    if (ret > 0) {
        TC_IOT_LOG_INFO("Request success, result:\n%s", buffer);
    } else {
        TC_IOT_LOG_INFO("request failed ,ret=%d", ret);
    }
    return 0;
}

