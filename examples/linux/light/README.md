# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md) ，创建产品和设备，注意事项：创建产品时，“鉴权模式”建议选择“临时token模式”；
2. 为产品定义以下3个可写数据模板：

| 名称    | 类型     | 读写 | 取值范围   |
| ---------- | ---------- | ---------- | ---------- |
| device_switch | 布尔 | 可写 | 无需填写 |
| color | 枚举 | 可写 | red,green,blue |
| brightness | 数值 | 可写 | 0,100 |
| power | 数值 | 只读 | 1,100 |


3. 点击导出按钮，导出 iot-xxxxx.json 数据模板描述文档，将 iot-xxxxx.json 文档放到 examples/linux/light 目录下覆盖 iot-product.json 文件。
4. 通过脚本自动生成 light 设备的逻辑框架及业务数据配置代码。

```shell
# 进入工具脚本目录
cd tools
python tc_iot_code_generator.py -c ../examples/linux/light/iot-product.json code_templates/tc_iot_device_*
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/linux/light/iot-xxxxx.json 文件成功
文件 ../examples/linux/light/tc_iot_device_config.h 生成成功
文件 ../examples/linux/light/tc_iot_device_logic.c 生成成功
文件 ../examples/linux/light/tc_iot_device_logic.h 生成成功
```

5. 代码及配置生成成功后，进入 build 目录，开始编译。

```shell
cd ../build
make
```


## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 light 程序。

```shell
# MQTT 直连并开启详细日志模式，运行 light 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s secret_abc  认证模式为Token模式时，-s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./light --trace -p 1883

./light -d device_xxxx -s secret_abc --trace -p 1883

# 如 light_device 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./light -d device_xxxx

```

## 模拟APP控制端发送控制指令
在控制台的【设备管理】中，点击【下发指令】，即可修改控制参数，下发推送给设备。


