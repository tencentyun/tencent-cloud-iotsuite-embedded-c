#ifndef TC_IOT_CONFIG_H
#define TC_IOT_CONFIG_H


#define TC_IOT_SDK_VERSION     "2.0.0"

/* mqtt消息接收缓冲区长度 */
#define TC_IOT_CLIENT_SEND_BUF_SIZE 1024
/* mqtt消息发送缓冲区长度 */
#define TC_IOT_CLIENT_READ_BUF_SIZE 1024

/* 网络异常或超时重连配置 */
/* 最小等待时间 */
#define TC_IOT_MIN_RECONNECT_WAIT_INTERVAL 1000
/* 最大等待时间 */
#define TC_IOT_MAX_RECONNECT_WAIT_INTERVAL 60000

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

#endif /* end of include guard */
