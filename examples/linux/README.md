# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md) ，创建产品和设备，注意事项：创建产品时，“鉴权模式”建议选择“临时token模式”；
2. 点击导出按钮，导出 iot-xxxxx.json 数据模板描述文档，将 iot-xxxxx.json 文档放到 examples/linux/ 目录下覆盖 iot-product.json 文件。
3. 通过脚本自动生成 mqtt 演示配置文件。

```shell
# 进入工具脚本目录
cd tools
./tc_iot_code_generator.py -c ../examples/linux/iot-product.json code_templates/tc_iot_device_config.h
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/linux/app/iot-product.json 文件成功
文件 ../examples/linux/app/tc_iot_device_config.h 生成成功
```

打开 tc_iot_device_config.h ，可以看到生成的如下产品相关信息：
```c
...
/* 设备激活及获取 secret 接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/secret */
/* Token接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/token */
/* 机房标识：
    广州机房=gz
    北京机房=bj
    ...
*/
#ifdef ENABLE_TLS
#define TC_IOT_CONFIG_AUTH_API_URL "https://gz.auth-device-iot.tencentcloudapi.com/token"
#define TC_IOT_CONFIG_ACTIVE_API_URL "https://gz.auth-device-iot.tencentcloudapi.com/secret"
#else
#define TC_IOT_CONFIG_AUTH_API_URL "http://gz.auth-device-iot.tencentcloudapi.com/token"
#define TC_IOT_CONFIG_ACTIVE_API_URL "http://gz.auth-device-iot.tencentcloudapi.com/secret"
#endif

#define TC_IOT_CONFIG_ACTIVE_API_URL_DEBUG   "http://gz.auth.iot.cloud.tencent.com/secret"
#define TC_IOT_CONFIG_AUTH_API_URL_DEBUG	 "http://gz.auth.iot.cloud.tencent.com/token"
...
#define TC_IOT_CONFIG_SERVER_HOST "mqtt-xxxx.ap-guangzhou.mqtt.tencentcloudmq.com"
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-yyyy"
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "mqtt-zzzz"
...
```

4. 修改 tc_iot_device_config.h 配置，设置 Device Name 和 Device Secret：
```c
/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"
```

5. 代码及配置生成成功后，进入 build 目录，开始编译。

```shell
cd ../build
make
```


## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 demo_mqtt 程序。

```shell
# MQTT 直连并开启详细日志模式，运行 demo_mqtt 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s secret_abc  认证模式为Token模式时，-s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./demo_mqtt --trace -p 1883

./demo_mqtt -d device_xxxx -s secret_abc --trace -p 1883

# 如 demo_mqtt_device 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./demo_mqtt -d device_xxxx

```

## MQTT 收发消息
收发 MQTT 消息，参见 demo_mqtt.c 中tc_iot_mqtt_client_publish(发送消息) & tc_iot_mqtt_client_subscribe(订阅 Topic) 。



