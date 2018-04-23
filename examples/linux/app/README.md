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


## 自定义设备数据和设备属性上报逻辑
### 设备数据
#### 关键变量
SDK 为设备定义了以下3个本地全局变量：
- g_tc_iot_device_local_data：本地数据最新状态，以下简称： local
    数据。
- g_tc_iot_device_desired_data：服务端最近的控制指令状态，SDK 内部使用，以下简称：desired 数据。
- g_tc_iot_device_reported_data：本地最近上报的数据最新状态，SDK
    内部使用，以下简称：reported 数据。

#### 处理逻辑
- 业务逻辑中，数据状态发生变化时，对应修改 local 数据字段，以反应当前最新的设备数据状态。比如，开关关闭，将开关对应字段改为 false，开发开启时，将对应的字段修改为 true；传感器监控的温湿度发生变化，将最新的温湿度信息数值，写入到温湿度对应的字段。
    1. 在 _tc_iot_property_change 这个回调内部处理后，返回了
       TC_IOT_SUCCESS 时，SDK 会主动将服务端下发的控制数据，写入到 local
       数据中，用户无需自行处理。如果用户不希望 SDK 改动 local 数据，
       可在函数处理结束时，返回 TC_IOT_FAILURE 。
    2. 其他情况，例如，自定义的循环检测、中断处理等过程，检测到数据变化时，将最新数据同步写入 local 数据后，需调用 tc_iot_report_device_data 来主动将数据上报服务端。
- 所有属性的回调 _tc_iot_property_change 处理完成后，SDK 会调用 tc_iot_confirm_devcie_data 检测 local 数据的变化情况，将状态上报给服务端。例如：在回调用户逻辑后，SDK 会比较 desired 和 local 的差异。如果数据一致，则判定服务端下发的控制指令处理成功，SDK 会自动上报到服务端，通知服务端清除对应的指令；SDK 同时会比较 reported 和 local 的差异，如果数据不一致，则将最新的状态，从 local 数据同步到 reported 数据中，并上报到服务端。

### 设备属性
tc_iot_device_logic.c 中 tc_iot_report_firm 函数负责上报设备属性信息，可以根据业务实际需要，修改为实际所需上报的字段和取值，最多允许上报5个属性，属性取值必须为字符串类型；


## 编译程序
代码及配置生成成功后，进入 build 目录，开始编译。

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

## 数据及函数执行流程
- 下图展示的流程为：设备离线时，控制端下发指令；设备上线后，读取指令数据并上报及清除指令的流程。

![图例](https://raw.githubusercontent.com/tencentyun/tencent-cloud-iotsuite-embedded-c/master/docs/iot-control-process.svg?sanitize=true)

- 下图展示的流程为：设备在线时，控制端下发指令；服务端直接推送指令。

![图例](https://raw.githubusercontent.com/tencentyun/tencent-cloud-iotsuite-embedded-c/master/docs/iot-control-process-online.svg?sanitize=true)
