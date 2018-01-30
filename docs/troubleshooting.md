# 常见错误解决及处理
## 常见错误码列表
### -1004 
错误码定义：TC_IOT_JSON_PARSE_FAILED = -1004

说明：JSON解析失败，一般为对端返回了非JSON格式的返回值，请抓包查看服务端的返回值是否正常。

### -1006 
错误码定义： TC_IOT_REFRESH_TOKEN_FAILED = -1006

说明：请求token（即MQTT的usernanme和password）响应失败，请检查 tc_iot_device_config.h 配置是否正确。
```c
 // MQ服务地址，可以在产品“基本信息页”->“mqtt链接地址”位置找到。
 #define TC_IOT_CONFIG_SERVER_HOST "mqtt-xxxxxxx.ap-guangzhou.mqtt.tencentcloudmq.com"
 // 产品id，可以在产品“基本信息页”->“产品id”位置找到
 #define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-xxxxxx"
 
 // 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到
 #define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"
 
 // 设备名称，可以在产品“设备管理”->“设备名称”位置找到
 #define TC_IOT_CONFIG_DEVICE_NAME "device_xxxx"
```
更多信息请参考：[开发环境配置](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md#%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83)

