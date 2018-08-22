# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md) ；
2. 创建产品，“数据协议”选择“数据模板”，进入【数据模板】页，为产品定义数据模板，可根据实际应用场景，自行定义所需的数据点及数据类型。例如，本示例默认定义了如下参数；

| 名称         | 类型       | 读写       | 取值范围             |
| ----------   | ---------- | ---------- | ----------           |
| param_bool   | 布尔       | 可写       | 无需填写             |
| param_enum   | 枚举       | 可写       | enum_a,enum_b,enum_c |
| param_number | 数值       | 可写       | 0,4095               |
| param_string | 字符串     | 可写       | 64                   |

3. 进入【基本信息】，点击【导出】，导出 iot-xxxxx.json 文档，将 iot-xxxxx.json 文档放到 examples/advanced_edition/http 目录下，覆盖 iot-product.json 文件。
4. 通过脚本自动生成设备的逻辑框架及业务数据配置代码。

```shell
# 进入工具脚本目录
cd tools
python tc_iot_code_generator.py -c ../examples/advanced_edition/http/iot-product.json code_templates/tc_iot_device_config.h
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/advanced_edition/http/iot-product.json 文件成功
文件 ../examples/advanced_edition/http/tc_iot_device_config.h 生成成功

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

## 模拟APP控制端发送控制指令
在控制台的【设备管理】中，点击【下发指令】，即可修改控制参数，下发推送给设备。

## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 advanced_http 程序。

```shell
# 运行 advanced_http 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_http_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./advanced_http --trace -p 1883

./advanced_http -d device_xxxx -s secret_abc --trace

# 如运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./advanced_http

```


