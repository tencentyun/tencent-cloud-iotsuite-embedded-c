# 如何运行本 Demo
## 准备工作
参见 (开发准备)[https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md] ，创建一个设备，配置 tc_iot_device_config.h 。

## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 light_device 程序。

```shell
# MQTT 直连并开启详细日志模式，运行本 demo
# 此种方式运行，可以有效查看日志及抓包定位问题
./light_device --trace -p 1883

# 如 light_device 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./light_device

```

## 模拟APP控制端发送控制指令
在 build/bin 目录下，通过脚本 tc_iot_shadow_cli.py 可以用来发送更新影子的指令。

```shell
# 获取调用接口的密钥信息：
# secret_id & secret_key
# https://console.qcloud.com/capi

# 获取产品ID和设备名称信息：
# prodcut_id=产品 ID & device_name=设备名称
# https://console.qcloud.com/iotsuite/product

# 更新影子数据
./tc_iot_shadow_cli.py UpdateIotShadow --product_id=iot-abc --device_name=device_abc -u secret_id -p secret_key --shadow={"desired":{"name":"light abc","color":256,"brightness":89,"light_switch":true}}

# 查询影子数据
./tc_iot_shadow_cli.py GetIotShadow --product_id=iot-abc --device_name=device_abc -u secret_id -p secret_key

```


