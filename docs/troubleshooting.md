# 常见错误解决及处理
## 详细定义
[tc_iot_const.h](https://tencentyun.github.io/tencent-cloud-iotsuite-embedded-c/sdk/tc__iot__const_8h.html)

## -101

- 说明

```c
TC_IOT_NET_UNKNOWN_HOST = -101
```

SDK 在通过网络层连接服务器的过程中，服务器域名解析失败。

- 诊断建议

1. 检查服务器地址配置是否正确；
2. 通过 ping 等工具，分析域名解析结果；

## -102

- 说明

```c
TC_IOT_NET_CONNECT_FAILED = -102
```

SDK 在通过网络层连接服务器的过程中，连接到服务器指定端口失败。

- 诊断建议

1. 检查服务器地址配置是否正确；
2. 通过 ping/telnet 等工具，检查网络是否正常，是否被防火墙隔离等；


## -161

- 说明

```c
TC_IOT_MQTT_CONNACK_PROTOCOL_UNACCEPTABLE = -161
```

SDK 在网络连接建立成功后，发起 MQTT CONN 请求，收到了 CONNACK 返回的错误码，指
示服务端当前不支持该 MQTT 协议版本。

- 诊断建议

1. 检查协议版本参数字段 MQTTVersion，一般取值应该是 4 (即 MQTT V3.1.1)；


## -162

- 说明

```c
TC_IOT_MQTT_CONNACK_CLIENT_ID_INVALID = -162
```

SDK 在网络连接建立成功后，发起 MQTT CONN 请求，收到了 CONNACK 返回的错误码，指
示 client id 参数非法。

- 诊断建议

1. 检查 client id 参数配置的值：

```c
// client id，
// 由两部分组成，组成形式为“Instanceid@DeviceID”，ClientID 的长度不超过 64个字符
// ，请不要使用不可见字符。其中
// Instanceid 为 IoT MQ 的实例 ID。
// DeviceID 为每个设备独一无二的标识，由业务方自己指定，需保证全局唯一，例如每个
// 传感器设备的序列号。
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID "mqtt-1e8w58ou4@" TC_IOT_CONFIG_DEVICE_NAME
```

## -163

- 说明

```c
TC_IOT_MQTT_CONNACK_SERVICE_UNAVAILABLE = -163
```

SDK 在网络连接建立成功后，发起 MQTT CONN 请求，收到了 CONNACK 返回的错误码，指
示服务不可用。

- 诊断建议

1. 联系 MQ 服务技术支持，确认服务状态。


## -164

- 说明

```c
TC_IOT_MQTT_CONNACK_BAD_USER_OR_PASSWORD = -164
```

SDK 在网络连接建立成功后，发起 MQTT CONN 请求，收到了 CONNACK 返回的错误码，指
示用户名密码数据格式无效。

- 诊断建议

1. 如果当前是通过固定用户名&密码方式访问MQ，请检查 username 和 password 配置，
看是否在拷贝过程中，丢失或多拷贝了非法字符。

```c
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
```

## -165

- 说明

```c
    TC_IOT_MQTT_CONNACK_NOT_AUTHORIZED = -165
```

SDK 在网络连接建立成功后，发起 MQTT CONN 请求，收到了 CONNACK 返回的错误码，指
示用户名密码校验不通过。

- 诊断建议

1. 如果当前是通过固定用户名&密码方式访问MQ，请检查 username 和 password 配置，
看是否在拷贝过程中，丢失或多拷贝了字符；或者是误用其他实例的用户名密码。

```c
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
```

## -170

- 说明

```c
TC_IOT_MQTT_SUBACK_FAILED = -170,  /**< 订阅失败，有可能 Topic 不存在，或者无权限等*/
```

SDK 发起 SUBSCRIBE 请求，收到了  SUBACK 返回的错误码，指示用户无权订阅该 Topic。

- 诊断建议

1. 检查订阅的 Topic 名称正确性，不能订阅不存在的 Topic，也不能订阅 其他产品或设
   备下的 Topic。

## -1004 

- 说明

JSON 解析失败，一般为请求了错误的地址或接口，或者服务端出现故障，返回了非 JSON
格式数据。

- 诊断建议

通过网络或日志打印，查看待解析的原始字符串，检查数据格式，看否是完整有效的 JSON 格式字符串。


## -1006 
### 说明
请求token（即MQTT的usernanme和password）响应失败，请检查tc_iot_device_config.h 中，如下配置是否正确。

```c

// 服务端获取动态username和password接口
// 地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/device
// 广州=gz
// 北京=bj
// ...
// 以下为广州机房的接口地址：
#ifdef ENABLE_TLS
#define TC_IOT_CONFIG_AUTH_API_URL "https://gz.auth-device-iot.tencentcloudapi.com/device"
#else
#define TC_IOT_CONFIG_AUTH_API_URL "http://gz.auth-device-iot.tencentcloudapi.com/device"
#endif

 // 产品id，可以在产品“基本信息页”->“产品id”位置找到
 #define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-xxxxxx"
 
 // 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到
 #define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"
 
 // 设备名称，可以在产品“设备管理”->“设备名称”位置找到
 #define TC_IOT_CONFIG_DEVICE_NAME "device_xxxx"
```

更多信息请参考：[开发环境配置
](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md#%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83)
