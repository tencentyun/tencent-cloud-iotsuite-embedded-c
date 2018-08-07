#ifndef TC_IOT_COAP_DEVICE_CONFIG_H
#define TC_IOT_COAP_DEVICE_CONFIG_H

/************************************************************************/
/**********************************必填项********************************/

#ifdef ENABLE_DTLS
/* 是否启用DTLS用于CoAP请求*/
#define TC_IOT_CONFIG_USE_DTLS 1
#else
#define TC_IOT_CONFIG_USE_DTLS 0
#endif

#if TC_IOT_CONFIG_USE_DTLS
/* CoAP服务的DTLS端口一般为5684*/
#define TC_IOT_CONFIG_COAP_SERVER_PORT 5684
#else
/* MQ服务的默认端口一般为5683*/
#define TC_IOT_CONFIG_COAP_SERVER_PORT 5683
#endif


/* 以下配置需要先在官网创建产品和设备，然后获取相关信息更新*/
#define TC_IOT_CONFIG_COAP_SERVER_HOST "gz.coap.tencentcloudapi.com"
/* 产品id，可以在产品“基本信息页”->“产品id”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-dlz9ojsy"
/* client id 由两部分组成，组成形式为“ProductKey@DeviceName” */
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "mqtt-1e8vngcm2"
/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"
/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"

#define  TC_IOT_PUB_TOPIC_PARM_FMT   "tp=/%s/%s/update"

/* client id 由两部分组成，组成形式为“ProductKey@DeviceName” */
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "@" TC_IOT_CONFIG_DEVICE_NAME

//#define TC_IOT_PUB_TOPIC_DEF TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME "/cmd"



#define TC_IOT_COAP_DTLS_PSK "secretPSK"
#define TC_IOT_COAP_DTLS_PSK_ID TC_IOT_CONFIG_DEVICE_NAME

/* TLS 握手执行超时时长，单位是毫秒*/
#define TC_IOT_CONFIG_DTLS_HANDSHAKE_TIMEOUT_MS  10000

/* tls 相关配置*/
/* 根证书路径*/
#define TC_IOT_CONFIG_ROOT_CA NULL
/* 客户端证书路径*/
#define TC_IOT_CONFIG_CLIENT_CRT NULL
/* 客户端私钥路径*/
#define TC_IOT_CONFIG_CLIENT_KEY NULL

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

#endif /* end of include guard */
