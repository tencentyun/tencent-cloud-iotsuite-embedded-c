#ifndef TC_IOT_DEVICE_CONFIG_H
#define TC_IOT_DEVICE_CONFIG_H

#include "tc_iot_config.h"

/**********************************必填项区域 begin ********************************/
/* 设备激活及获取 secret 接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/secret */
/* Token接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/token */
/* 机房标识：
    广州机房=gz
    北京机房=bj
    ...
*/
#define TC_IOT_CONFIG_REGION  "gz"

/* 以下配置需要先在官网创建产品和设备，然后获取相关信息更新*/
/* MQ服务地址，可以在产品“基本信息”->“mqtt链接地址”位置找到。*/
#define TC_IOT_CONFIG_MQ_SERVER_HOST "mqtt-ify9tmhk.ap-guangzhou.mqtt.tencentcloudmq.com"

/* 产品id，可以在产品“基本信息”->“产品id”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-gohk1kbq"

/* 产品id，可以在产品“基本信息”->“产品key”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "mqtt-ify9tmhk"

/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"

/* 鉴权模式，可以在产品的“基本信息”->“鉴权模式”位置找到
 * 1:动态令牌模式
 * 2:签名认证模式
 * */
#define TC_IOT_CONFIG_AUTH_MODE   1

/**********************************必填项区域 end ********************************/


/**********************************选填项区域 begin ********************************/
/*
 * 除非处于调试或特殊应用场景，以下内容一般情况下不需要手动修改。
 * */

/*------------------MQTT begin---------------------*/
#ifdef ENABLE_TLS
// 请求 API 时使用的协议，https 或者 http
#define TC_IOT_CONFIG_API_HTTP_PROTOCOL "https"

// MQ 服务的 MQTT 是否通过 TLS 协议通讯，1 为使用，0 表示不使用
#define TC_IOT_CONFIG_USE_TLS 1 
/* MQ服务的TLS端口一般为8883*/
#define TC_IOT_CONFIG_MQ_SERVER_PORT 8883

#else
// 请求 API 时使用的协议，https 或者 http
#define TC_IOT_CONFIG_API_HTTP_PROTOCOL "http"

// MQ 服务的 MQTT 是否通过 TLS 协议通讯，1 为使用，0 表示不使用
#define TC_IOT_CONFIG_USE_TLS 0
/* MQ服务的默认端口一般为1883*/
#define TC_IOT_CONFIG_MQ_SERVER_PORT 1883
#endif

/* client id 由两部分组成，组成形式为“ProductKey@DeviceName” */
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "@" TC_IOT_CONFIG_DEVICE_NAME


/* 关于username和password：*/
/* 1)如果是通过TC_IOT_CONFIG_AUTH_API_URL接口，动态获取的，以下两个参数可不用填写*/
/* 2)如果有预先申请好的固定username和password，可以把获取到的固定参数填写到如下位置*/
#if TC_IOT_CONFIG_AUTH_MODE == 1
/* 动态令牌模式 */
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
#define TC_IOT_AUTH_FUNC   tc_iot_refresh_auth_token
#elif TC_IOT_CONFIG_AUTH_MODE == 2
/*签名认证模式*/
#define TC_IOT_AUTH_FUNC   tc_iot_mqtt_refresh_dynamic_sign
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
#else
/* 直连模式 */
#define TC_IOT_AUTH_FUNC(a,b,c,d)   
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
#endif

// begin: DEBUG use only 
#define TC_IOT_CONFIG_ACTIVE_API_URL_DEBUG   "http://" TC_IOT_SERVER_REGION ".auth.iot.cloud.tencent.com/secret"
#define TC_IOT_CONFIG_AUTH_API_URL_DEBUG	 "http://" TC_IOT_SERVER_REGION ".auth.iot.cloud.tencent.com/token"
// end: DEBUG use only 

// API 服务域名根地址
#define TC_IOT_CONFIG_API_DOMAIN_ROOT TC_IOT_CONFIG_API_HTTP_PROTOCOL "://" TC_IOT_CONFIG_REGION ".auth-device-iot.tencentcloudapi.com"

// 设备动态激活请求地址
#define TC_IOT_CONFIG_ACTIVE_API_URL  TC_IOT_CONFIG_API_DOMAIN_ROOT "/secret"

// 设备动态令牌请求地址
#define TC_IOT_CONFIG_AUTH_API_URL    TC_IOT_CONFIG_API_DOMAIN_ROOT "/token"

/* connect、publish、subscribe、unsubscribe */
/* 等命令执行超时时长，单位是毫秒*/
#define TC_IOT_CONFIG_COMMAND_TIMEOUT_MS  10000
/* TLS 握手执行超时时长，单位是毫秒*/
#define TC_IOT_CONFIG_TLS_HANDSHAKE_TIMEOUT_MS  10000
/* keep alive 间隔时长，单位是秒*/
#define TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC  60
/* 网络故障或服务端超时，是否自动重连*/
#define TC_IOT_CONFIG_AUTO_RECONNECT 1

#define TC_IOT_CONFIG_CLEAN_SESSION 1

/* 自定义协议时，建议下行消息topic，mq服务端的响应和下行推送，*/
/* 都会发布到 "<product id>/<device name>/cmd" 这个topic，此 Topic
 * 需要用户使用前，自行在“Topic 类管理”中创建*/
/* 客户端只需要订阅这个topic即可*/
#define TC_IOT_MQTT_SUB_TOPIC_FMT "%s/%s/cmd"
#define TC_IOT_MQTT_SUB_TOPIC_DEF TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME "/cmd"

/* 自定义协议时，建议上行消息topic，客户端请求服务端的消息，发到到这个topic即可*/
/* topic格式"<product id>/<device name>/update" 这个topic，此 Topic
 * 需要用户使用前，自行在“Topic 类管理”中创建*/
#define TC_IOT_MQTT_PUB_TOPIC_FMT "%s/%s/update"
#define TC_IOT_MQTT_PUB_TOPIC_DEF TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME "/update"

/* shadow下行消息topic，mq服务端的响应和下行推送，*/
/* 都会发布到 "shadow/get/<product id>/<device name>" 这个topic*/
/* 客户端只需要订阅这个topic即可*/
#define TC_IOT_SHADOW_SUB_TOPIC_PREFIX "shadow/get/"
#define TC_IOT_SHADOW_SUB_TOPIC_FMT TC_IOT_SHADOW_SUB_TOPIC_PREFIX "%s/%s"
#define TC_IOT_SHADOW_SUB_TOPIC_DEF TC_IOT_SHADOW_SUB_TOPIC_PREFIX TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME

/* shadow上行消息topic，客户端请求服务端的消息，发到到这个topic即可*/
/* topic格式"shadow/update/<product id>/<device name>"*/
#define TC_IOT_SHADOW_PUB_TOPIC_PREFIX "shadow/update/"
#define TC_IOT_SHADOW_PUB_TOPIC_FMT TC_IOT_SHADOW_PUB_TOPIC_PREFIX "%s/%s"
#define TC_IOT_SHADOW_PUB_TOPIC_DEF TC_IOT_SHADOW_PUB_TOPIC_PREFIX TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME

/*------------------MQTT end---------------------*/


/*------------------COAP begin---------------------*/

// COAP 上报地址，tp= 为固定参数名称前缀，/xxxx/yyyy/update 为实际 Topic
#define  TC_IOT_COAP_PUB_TOPIC_PARM_FMT   "tp=/%s/%s/update"

// COAP RPC 订阅下行地址，st= 为固定参数名称前缀，shadow/get/xxx 为实际 Topic
#define  TC_IOT_COAP_RPC_SUB_TOPIC_PARM_FMT   "st=shadow/get/%s/%s"

// COAP RPC 发送请求的上行地址，pt= 为固定参数名称前缀，shadow/get/xxx 为实际 Topic
#define  TC_IOT_COAP_RPC_PUB_TOPIC_PARM_FMT   "pt=shadow/update/%s/%s"

#define TC_IOT_COAP_DTLS_PSK "secretPSK"
#define TC_IOT_COAP_DTLS_PSK_ID TC_IOT_CONFIG_DEVICE_NAME


#define TC_IOT_CONFIG_COAP_SERVER_HOST TC_IOT_CONFIG_REGION ".coap.tencentcloudapi.com"

#ifdef ENABLE_DTLS
/* 是否启用DTLS用于CoAP请求*/
#define TC_IOT_CONFIG_USE_DTLS 1
#else
#define TC_IOT_CONFIG_USE_DTLS 0
#endif


/* TLS 握手执行超时时长，单位是毫秒*/
#define TC_IOT_CONFIG_DTLS_HANDSHAKE_TIMEOUT_MS  10000

#if TC_IOT_CONFIG_USE_DTLS
/* CoAP服务的DTLS端口一般为5684*/
#define TC_IOT_CONFIG_COAP_SERVER_PORT 5684
#else
/* MQ服务的默认端口一般为5683*/
#define TC_IOT_CONFIG_COAP_SERVER_PORT 5683
#endif

/*------------------COAP end---------------------*/

/*------------------HTTP begin---------------------*/
#define TC_IOT_CONFIG_RPC_API_URL  TC_IOT_CONFIG_API_DOMAIN_ROOT "/rpc"
/*------------------HTTP end---------------------*/

/*-----------------COMMON begin----------------------*/
/* tls 相关配置*/
/* 根证书路径*/
#define TC_IOT_CONFIG_ROOT_CA NULL
/* 客户端证书路径*/
#define TC_IOT_CONFIG_CLIENT_CRT NULL
/* 客户端私钥路径*/
#define TC_IOT_CONFIG_CLIENT_KEY NULL

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"
/*-----------------COMMON end----------------------*/

/**********************************选填项区域 end ********************************/

#endif /* end of include guard */
