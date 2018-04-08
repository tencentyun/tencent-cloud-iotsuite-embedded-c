# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md) ，创建产品和设备，
2. 在产品控制台的【基本信息页】中导出 iot-xxxxx.json 数据点描述文档，将 iot-xxxxx.json 文档放到 examples/linux/light 目录下。
3. 通过脚本自动生成 light 设备的逻辑框架及业务数据配置代码。
```shell
# 进入工具脚本目录
cd tools
# 注意将 iot-xxxxx.json 对应修改为实际文件名。
./tc_iot_code_generator.py -c ../examples/linux/light/iot-xxxxx.json code_templates/*
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/linux/light/iot-xxxxx.json 文件成功
文件 ../examples/linux/light/tc_iot_device_config.h 生成成功
文件 ../examples/linux/light/tc_iot_device_logic.c 生成成功
文件 ../examples/linux/light/tc_iot_device_logic.h 生成成功
```

4. 代码及配置生成成功后，进入 build 目录，开始编译。

```shell
cd ../build
make
```


## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 light 程序。

```shell
# MQTT 直连并开启详细日志模式，运行 light 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：-d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
# ./light --trace -p 1883

./light -d device_xxxx --trace -p 1883

# 如 light_device 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./light -d device_xxxx

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
./tc_iot_shadow_cli.py UpdateIotShadow --product_id=iot-abc --device_name=device_abc -u secret_id -p secret_key --shadow={"desired":{"name":"light abc","color":4,"brightness":89,"light_switch":true}}

# 查询影子数据
./tc_iot_shadow_cli.py GetIotShadow --product_id=iot-abc --device_name=device_abc -u secret_id -p secret_key

```

根据控制端的指令，可以看到 light_device 程序，输出的状态数据，对应发生的变化。
自动随机更新数据脚本，可参见 tools/light_controller.sh 。



