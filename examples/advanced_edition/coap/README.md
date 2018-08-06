# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md) ，创建产品和设备，注意事项：创建产品时，“鉴权模式”建议选择“临时token模式”，“数据协议”选择“数据模板”；
2. 通过【数据模板】功能，为产品定义数据模板；
3. 修改 tc_iot_coap_device_config.h 编译配置文件，配置产品信息相关参数，详见注释：
```shell
/* 以下配置需要先在官网创建产品和设备，然后获取相关信息更新*/
#define TC_IOT_CONFIG_COAP_SERVER_HOST "localhost"
/* 产品id，可以在产品“基本信息页”->“产品id”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-7hjcfc6k"
/* client id 由两部分组成，组成形式为“ProductKey@DeviceName” */
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
编译完成后，在 build/bin/ 目录下，会产生一个 demo_coap_client 程序。

```shell
# 开启详细日志模式，运行 demo_coap_client 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_coap_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s secret_abc  认证模式为Token模式时，-s 指定Device Secret
# 如果已经在 tc_iot_coap_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./demo_coap_client --trace -p 5683

./demo_coap_client -d device_xxxx -s secret_abc --trace -p 5683

# 如 demo_coap_client_device 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./demo_coap_client -d device_xxxx -p 5683

```


