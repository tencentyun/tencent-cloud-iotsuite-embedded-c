#ifndef IOT_DEVICE_CONFIG_H
#define IOT_DEVICE_CONFIG_H

#ifdef TEST
// 内网测试配置 忽略以下内容
#define TC_IOT_CONFIG_AUTH_API_URL "http://auth.iot.cloud.tencent.com/device"
#define TC_IOT_CONFIG_SERVER_HOST "<instanceid>.<location>.mqtt.myqcloud.com"
#define TC_IOT_CONFIG_SERVER_PORT 1883
#define TC_IOT_CONFIG_DEVICE_SECRET "<device secret>"
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "<product id>"
#define TC_IOT_CONFIG_DEVICE_NAME "<device name>"
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID "<Instanceid@client_unique_id>"
#define TC_IOT_CONFIG_DEVICE_USER_NAME "<user name>"
#define TC_IOT_CONFIG_DEVICE_PASSWORD "<password>"
// 内网测试配置 忽略以上内容

#else

// 服务端获取动态username和password接口，
// 此项配置一般固定不变，无需修改。
#ifdef ENABLE_TLS
#define TC_IOT_CONFIG_AUTH_API_URL \
    "https://auth-device-iot.tencentcloudapi.com/device"
#else
#define TC_IOT_CONFIG_AUTH_API_URL \
    "http://auth-device-iot.tencentcloudapi.com/device"
#endif

/************************************************************************/
/**********************************必填项********************************/
// 以下配置需要先在官网创建产品和设备，然后获取相关信息更新
// MQ服务地址，可以在产品“基本信息页”->“mqtt链接地址”位置找到。
#define TC_IOT_CONFIG_SERVER_HOST \
    "<mqtt-xxx.ap-guangzhou.mqtt.tencentcloudmq.com>"
// MQ服务端口，直连一般为1883、tls端口一般为8883
#define TC_IOT_CONFIG_SERVER_PORT 1883
// 产品id，可以在产品“基本信息页”->“产品id”位置找到
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "<iot-xxx>"

// 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到
#define TC_IOT_CONFIG_DEVICE_SECRET "<0000000000000000>"
// 设备名称，可以在产品“设备管理”->“设备名称”位置找到
#define TC_IOT_CONFIG_DEVICE_NAME "<device001>"
// client id，
// 由两部分组成，组成形式为“Instanceid@DeviceID”，ClientID 的长度不超过 64个字符
// ，请不要使用不可见字符。其中
// Instanceid 为 IoT MQ 的实例 ID。
// DeviceID 为每个设备独一无二的标识，由业务方自己指定，需保证全局唯一，例如每个
// 传感器设备的序列号。
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID "<mqtt-xxxx@device001>"

/************************************************************************/
/**********************************选填项********************************/
// 关于username和password：
// 1)如果是通过TC_IOT_CONFIG_AUTH_API_URL接口，动态获取的，以下两个参数可不用填写
// 2)如果有预先申请好的固定username和password，可以把获取到的固定参数填写到如下位置
#define TC_IOT_CONFIG_DEVICE_USER_NAME "<user name>"
#define TC_IOT_CONFIG_DEVICE_PASSWORD "<******>"
/************************************************************************/

#endif

// connect、publish、subscribe、unsubscribe
// 等命令执行超时时长，单位是毫秒
#define TC_IOT_CONFIG_COMMAND_TIMEOUT_MS 2000
// keep alive 间隔时长，单位是秒
#define TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC 60
// 网络故障或服务端超时，是否自动重连
#define TC_IOT_CONFIG_AUTO_RECONNECT 0

#define TC_IOT_CONFIG_CLEAN_SESSION 0
// 是否启用tls用于mqtt或http（即https）请求
#define TC_IOT_CONFIG_USE_TLS 0

// shadow下行消息topic，mq服务端的响应和下行推送，
// 都会发布到 "shadow/get/<product id>/<device name>" 这个topic
// 客户端只需要订阅这个topic即可
#define SUB_TOPIC \
    "shadow/get/" TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME
// shadow上行消息topic，客户端请求服务端的消息，发到到这个topic即可
// topic格式"shadow/update/<product id>/<device name>"
#define PUB_TOPIC                                    \
    "shadow/update/" TC_IOT_CONFIG_DEVICE_PRODUCT_ID \
    "/" TC_IOT_CONFIG_DEVICE_NAME

// tls 相关配置：暂不支持tls，
// 根证书路径
#define TC_IOT_CONFIG_ROOT_CA "<path to rootCA.crt>"
// 客户端证书路径
#define TC_IOT_CONFIG_CLIENT_CRT "<path to client crt>"
// 客户端私钥路径
#define TC_IOT_CONFIG_CLIENT_KEY "<path to client key>"

#endif /* end of include guard */
