# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md) ，创建产品和设备，注意事项：创建产品时，“鉴权模式”建议选择“临时token模式”；
2. 通过【基本信息】页的【数据模板】功能，为产品定义数据模板；
3. 点击导出按钮，导出 iot-xxxxx.json 数据模板描述文档，将 iot-xxxxx.json 文档放到 examples/linux/app 目录下覆盖 iot-product.json 文件。
4. 通过脚本自动生成 iotsuite_app 设备的逻辑框架及业务数据配置代码。

```shell
# 进入工具脚本目录
cd tools
./tc_iot_code_generator.py -c ../examples/linux/app/iot-product.json code_templates/*
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/linux/app/iot-product.json 文件成功
文件 ../examples/linux/app/tc_iot_device_config.h 生成成功
文件 ../examples/linux/app/tc_iot_device_logic.c 生成成功
文件 ../examples/linux/app/tc_iot_device_logic.h 生成成功
```

5. 自定义设备数据和设备属性上报逻辑：
- 设备数据：设备数据发生变化时，需要同步修改 g_tc_iot_device_local_data 对应的字段，并调用 tc_iot_report_device_data 将数据上报服务端；
- 属性信息：tc_iot_device_logic.c 中 tc_iot_report_firm 函数负责上报设备属性信息，可以根据业务实际需要，修改为实际所需上报的字段和取值，最多允许上报5个属性，属性取值必须为字符串类型；

6. 代码及配置生成成功后，进入 build 目录，开始编译。

```shell
cd ../build
make
```


## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 iotsuite_app 程序。

```shell
# MQTT 直连并开启详细日志模式，运行 iotsuite_app 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s secret_abc  认证模式为Token模式时，-s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./iotsuite_app --trace -p 1883

./iotsuite_app -d device_xxxx -s secret_abc --trace -p 1883

# 如 iotsuite_app_device 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./iotsuite_app -d device_xxxx

```

## 模拟APP控制端发送控制指令
在控制台的【设备管理】中，点击【下发指令】，即可修改控制参数，下发推送给设备。


