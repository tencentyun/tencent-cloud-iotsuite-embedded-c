# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md)； 
2. 创建产品，“数据协议”选择“自定义”，创建如下自定义Topic：
    - ${product_id}/${device_name}/cmd ，用于设备端接收指令。
    - ${product_id}/${device_name}/update ，用于设备端上报数据。

3. 进入【基本信息】，点击【导出】，导出 iot-xxxxx.json 文档，将 iot-xxxxx.json 文档放到 examples/scenarios/smartbox 目录下，覆盖 iot-product.json 文件。
4. 通过脚本自动生成演示配置文件。

```shell
# 进入工具脚本目录
cd tools
python tc_iot_code_generator.py -c ../examples/scenarios/smartbox/iot-product.json code_templates/tc_iot_device_config.h
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/scenarios/smartbox/iot-product.json 文件成功
文件 ../examples/scenarios/smartbox/tc_iot_device_config.h 生成成功
```

5. 修改 tc_iot_device_config.h 配置，设置 Device Name 和 Device Secret：
```c
/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"
```

## 编译程序
代码及配置生成成功后，进入 build 目录，开始编译。

```shell
cd ../build
make
```


## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 scn_smartbox 程序。

```shell
# 运行 scn_smartbox 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./scn_smartbox --trace -p 1883

./scn_smartbox -d device_xxxx -s secret_abc --trace -p 1883

# 如 scn_smartbox_device 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./scn_smartbox -d device_xxxx

```

## 模拟APP控制端发送控制指令
进入控制台的设备管理页面，使用下发指令功能向 ${product_id}/${device_name}/cmd Topic 发送消息，即可控制设备。


