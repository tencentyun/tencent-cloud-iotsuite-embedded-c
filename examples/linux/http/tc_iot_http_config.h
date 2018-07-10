#ifndef TC_IOT_HTTP_CONFIG_H
#define TC_IOT_HTTP_CONFIG_H

/* 设备激活及获取 secret 接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/secret */
/* Token接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/token */
/* 机房标识：
    广州机房=gz
    北京机房=bj
    ...
*/
#define TC_IOT_CONFIG_API_REGION     "gz"

#ifdef ENABLE_TLS
#define TC_IOT_CONFIG_API_HTTP_PROTOCOL "https"
#else
#define TC_IOT_CONFIG_API_HTTP_PROTOCOL "http"
#endif

#define TC_IOT_CONFIG_API_DOMAIN_ROOT TC_IOT_CONFIG_API_HTTP_PROTOCOL "://" TC_IOT_CONFIG_API_REGION ".auth-device-iot.tencentcloudapi.com"

#define TC_IOT_CONFIG_RPC_API_URL  TC_IOT_CONFIG_API_DOMAIN_ROOT "/rpc"


/* 以下配置需要先在官网创建产品和设备，然后获取相关信息更新*/
/* MQ服务地址，可以在产品“基本信息页”->“mqtt链接地址”位置找到。*/
#define TC_IOT_CONFIG_SERVER_HOST "mqtt-5ns8xh714.ap-guangzhou.mqtt.tencentcloudmq.com"
/*#define TC_IOT_CONFIG_SERVER_HOST "localhost"*/
/* 产品id，可以在产品“基本信息页”->“产品id”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-7hjcfc6k"
/* 产品id，可以在产品“基本信息页”->“产品key”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "mqtt-5ns8xh714"

/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "light001"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

#endif /* end of include guard */
