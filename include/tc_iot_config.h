#ifndef TC_IOT_CONFIG_H
#define TC_IOT_CONFIG_H

#include "tc_iot_compile_flags.h"


/* buffer 长度的倍数因子, 当属性太多json太长的时候, 一些buffer长度需要增加, 可以设置成 2 , 4 ,8 等数值 */
#define BUF_MULTIPLE 			(1)

#define TC_IOT_SDK_VERSION     "2.8"

/* mqtt消息接收缓冲区长度 */
#define TC_IOT_CLIENT_READ_BUF_SIZE (1024*BUF_MULTIPLE)
/* mqtt消息发送缓冲区长度 */
#define TC_IOT_CLIENT_SEND_BUF_SIZE (1024*BUF_MULTIPLE)

/* 网络异常或超时重连配置 */
/* 最小等待时间 */
#define TC_IOT_MIN_RECONNECT_WAIT_INTERVAL 1000
/* 最大等待时间 */
#define TC_IOT_MAX_RECONNECT_WAIT_INTERVAL 60000

/* 接收到 MQTT 包头以后，接收剩余长度及剩余包，最大延迟等待时延 */
#define TC_IOT_MQTT_MAX_REMAIN_WAIT_MS     2000

/* 设备名称长度 */
#define TC_IOT_MAX_DEVICE_NAME_LEN 20

/* 设备信息名称长度 */
#define TC_IOT_MAX_FIRM_INFO_NAME_LEN 256

/* 设备信息取值长度 */
#define TC_IOT_MAX_FIRM_INFO_VALUE_LEN 256

/* clientid和topic的定义与限制： */
/* https://cloud.tencent.com/document/product/646/12657 */
/* client id 最大长度 */
#define TC_IOT_MAX_CLIENT_ID_LEN 64
/* 订阅topic最大长度 */
#define TC_IOT_MAX_MQTT_TOPIC_LEN 128

/*
 * 基本长度：{"method":"get","passthrough":{"sid":"3ffe0001"},"metadata":false}
 * */
#define TC_IOT_GET_MSG_LEN   96

/*
 * 基本长度：{"method":"update","passthrough":{"sid":"3ffe0001"},"state":{"reported":{}}}
 *    +
 * 数据点属性长度*数据点数
 * */
#define TC_IOT_REPORT_UPDATE_MSG_LEN   (1024*BUF_MULTIPLE)

/**
 * 基本长度：{"method":"delete","passthrough":{"sid":"40b20006"},"state":{"desired":{}}}
 *    +
 * 数据点属性长度*数据点数
 * */
#define TC_IOT_UPDATE_DESIRED_MSG_LEN   (1024*BUF_MULTIPLE)

/**
 * 基本长度：{"method":"report_firm","payload":{"sdk-ver":"2.5","firm-ver":"LINUXV1.0"}}
 *
 */

#define TC_IOT_REPORT_FIRM_MSG_LEN  128

/**
 * 基本长度：{"method":"report_upgrade","passthrough":{"mid":1},"payload":{"ota_id":"ota-k7pz1fxo_0","ota_code":"1","ota_status":"OTA
 * command received","ota_message":"success"}}
 *
 */
#define TC_IOT_REPORT_UPGRADE_MSG_LEN  256

/* username和password计算规则： */
/* https://cloud.tencent.com/document/product/646/12661 */
/* 用户名长度 */
#define TC_IOT_MAX_USER_NAME_LEN 128
/* 密码长度 */
#define TC_IOT_MAX_PASSWORD_LEN 128
/* 签名秘钥长度 */
#define TC_IOT_MAX_SECRET_LEN 64
/* product id 长度 */
#define TC_IOT_MAX_PRODUCT_ID_LEN 20
/* product key 长度 */
#define TC_IOT_MAX_PRODUCT_KEY_LEN 20

/* TLS 握手超时时长 */
#define TC_IOT_DEFAULT_TLS_HANSHAKE_TIMEOUT_MS  10000

// HTTPS 请求时，是否严格检查服务端证书
#define TC_IOT_HTTPS_CERT_STRICT_CHECK       0

// 请求 激活设备 Active 接口表单长度
#define TC_IOT_HTTP_ACTIVE_REQUEST_FORM_LEN  256
// 请求 激活设备 Active 接口总长度
#define TC_IOT_HTTP_ACTIVE_REQUEST_LEN       1024
// 请求 激活设备 Active 接口响应数据
#define TC_IOT_HTTP_ACTIVE_RESPONSE_LEN      512  

#define TC_IOT_HTTP_OTA_REQUEST_LEN          1024

// 请求 Token 接口表单长度
#define TC_IOT_HTTP_TOKEN_REQUEST_FORM_LEN  256
// 请求 Token 接口响应数据
#define TC_IOT_HTTP_TOKEN_RESPONSE_LEN      512  

// 请求 Rpc 接口表单长度
#define TC_IOT_HTTP_MQAPI_REQUEST_FORM_LEN  (1024*BUF_MULTIPLE)
// 请求 Rpc 接口 HTTP 请求或响应最大长度
#define TC_IOT_HTTP_MQAPI_RESPONSE_LEN      (1024*BUF_MULTIPLE)


#define TC_IOT_MAX_JSON_TOKEN_COUNT         (120*BUF_MULTIPLE)

#define TC_IOT_TLS_ERROR_STR_LEN            64
#define TC_IOT_TLS_INFO_LEN                 64

#endif /* end of include guard */
