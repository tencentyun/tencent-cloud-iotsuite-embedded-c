#include "tc_iot_export.h"

#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-7hjcfc6k"
/* 产品id，可以在产品“基本信息页”->“产品key”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "mqtt-5ns8xh714"

/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"


tc_iot_device_info g_device = {
    /* device info*/
    TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
    TC_IOT_CONFIG_DEVICE_NAME, "",
};

int main(int argc, char** argv) {
       
    int nonce = 1064325108;
    int timestamp = 1530784981;

    const char * message = "{\"method\":\"get\"}";

    /* int ret = http_mqapi_rpc("http://iotsuite.ioloiolo.com/rpc", NULL, timestamp, nonce, &g_device , message); */
    int ret = http_mqapi_rpc("http://gz.auth-device-iot.tencentcloudapi.com/rpc", NULL, timestamp, nonce, &g_device , message);
    printf("%d\n", ret);
    return 0;
}

