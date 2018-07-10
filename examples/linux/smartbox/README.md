# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md) ，创建产品和设备，注意事项：创建产品时，“数据协议”选择“自定义”；
2. 为产品定义创建如下2个自定义Topic：
    - ${product_id}/${device_name}/cmd ，用来接收云端指令。
    - ${product_id}/${device_name}/update ，用来发布设备上报消息。

3. 修改 tc_iot_device_config.h 编译配置文件，配置产品信息相关参数，详见注释：
```shell
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

/* 以下配置需要先在官网创建产品和设备，然后获取相关信息更新*/
/* MQ服务地址，可以在产品“基本信息页”->“mqtt链接地址”位置找到。*/
#define TC_IOT_CONFIG_SERVER_HOST "mqtt-5ns8xh714.ap-guangzhou.mqtt.tencentcloudmq.com"
/*#define TC_IOT_CONFIG_SERVER_HOST "localhost"*/
/* 产品id，可以在产品“基本信息页”->“产品id”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-7hjcfc6k"
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "mqtt-5ns8xh714"

/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"

```

4. 代码及配置生成成功后，进入 build 目录，开始编译。

```shell
cd build
make
```


## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 smartbox 程序。

```shell
# 运行 smartbox 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./smartbox --trace -p 1883

./smartbox -d device_xxxx -s secret_abc --trace -p 1883

# 如 smartbox_device 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./smartbox -d device_xxxx

```

## 模拟APP控制端发送控制指令
调用 [向Topic发布消息](https://cloud.tencent.com/document/product/568/16452) 接口，向 ${product_id}/${device_name}/cmd Topic 发送消息，即可控制。


