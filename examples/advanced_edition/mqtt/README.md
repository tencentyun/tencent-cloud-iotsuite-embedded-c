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

3. 进入【基本信息】，点击【导出】，导出 iot-xxxxx.json 文档，将 iot-xxxxx.json 文档放到 examples/advanced_edition/mqtt 目录下，覆盖 iot-product.json 文件。
4. 通过脚本自动生成设备的逻辑框架及业务数据配置代码。

```shell
# 进入工具脚本目录
cd tools
python tc_iot_code_generator.py -c ../examples/advanced_edition/mqtt/iot-product.json code_templates/*
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/advanced_edition/mqtt/iot-product.json 文件成功
文件 ../examples/advanced_edition/mqtt/app_main.c 生成成功
文件 ../examples/advanced_edition/mqtt/tc_iot_device_config.h 生成成功
文件 ../examples/advanced_edition/mqtt/tc_iot_device_logic.c 生成成功
文件 ../examples/advanced_edition/mqtt/tc_iot_device_logic.h 生成成功

```

5. 修改 tc_iot_device_config.h 配置，设置 Device Name 和 Device Secret：
```c
/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"
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


## 编译程序
代码及配置生成成功后，进入 build 目录，开始编译。

```shell
cd ../build
make
```

## 运行程序

编译完成后，在 build/bin/ 目录下，会产生一个 advanced_mqtt 程序。

```shell
# 运行 advanced_mqtt 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./advanced_mqtt --trace -p 1883

./advanced_mqtt -d device_xxxx -s secret_abc --trace -p 1883

# 如运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./advanced_mqtt -d device_xxxx

```

## 模拟APP控制端发送控制指令
在控制台的【设备管理】中，点击【下发指令】，即可修改控制参数，下发推送给设备。

## 数据交互流程
- 下图展示的流程为：设备离线时，控制端下发指令；设备上线后，读取指令数据并上报及清除指令的流程。

![图例](https://user-images.githubusercontent.com/990858/44081769-1cf0833a-9fe2-11e8-90ff-13b6fb62c7f6.png)

- 下图展示的流程为：设备在线时，控制端下发指令；服务端直接推送指令。

![图例](https://user-images.githubusercontent.com/990858/44081626-bf1e1ace-9fe1-11e8-9f82-031b23c6dcff.png)

## SDK API 样例及说明

### 1. 初始化 
tc_iot_server_init 根据设备配置参数，初始化服务。此函数调用后：
1. 初始化相关结构变量，和服务端建立 MQTT 连接；
2. 默认订阅数据模板相关 Topic；
3. 发起 get 请求，从服务端同步最新数据；

#### 样例

```c
    int ret = 0;
    ret = tc_iot_server_init(tc_iot_get_shadow_client(), &g_tc_iot_shadow_config);
```

#### 函数原型及说明

```c
/**
 *  @brief tc_iot_server_init
 * 根据设备配置参数，初始化服务。
 *  @param  p_shadow_client 设备服务对象
 *  @param  p_client_config 服务配置参数。
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */
int tc_iot_server_init(tc_iot_shadow_client* p_shadow_client, tc_iot_shadow_config * p_client_config);
```

### 2. 主循环 
tc_iot_server_loop 服务任务主循环函数，接收服务推送及响应数据。 此函数调用后：
1. 接收服务端下发的控制指令或响应消息，并调用业务回调函数处理；
2. 定时触发心跳逻辑，保持连接；
3. 检测网络连接状态，异常时自动重连；

#### 样例

```c
    while (!stop) {
        tc_iot_server_loop(tc_iot_get_shadow_client(), 200);
    }
```

#### 函数原型及说明

```c
/**
 *  @brief tc_iot_server_loop
 *  服务任务主循环函数，接收服务推送及响应数据。
 *  @param  p_shadow_client 设备服务对象
 *  @param yield_timeout 循环等待时间，单位毫秒
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */

int tc_iot_server_loop(tc_iot_shadow_client* p_shadow_client, int yield_timeout);
```

### 3. 析构资源
 tc_iot_server_destroy 数据模板服务析构处理，释放资源。

#### 样例

```c
    tc_iot_server_destroy(tc_iot_get_shadow_client());
```

#### 函数原型及说明

```c
/**
 *  @brief tc_iot_server_destroy
 * 数据模板服务析构处理，释放资源。
 *  @param  p_shadow_client 设备影子对象
 *  @return 结果返回码
 *  @see tc_iot_sys_code_e
 */

int tc_iot_server_destroy(tc_iot_shadow_client* p_shadow_client);
```
