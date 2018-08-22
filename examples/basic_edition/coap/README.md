# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md)； 
2. 创建产品，“数据协议”选择“自定义”，创建如下自定义Topic：
    - ${product_id}/${device_name}/update ，用于设备端上报数据。
3. 进入【基本信息】，点击【导出】，导出 iot-xxxxx.json 文档，将 iot-xxxxx.json 文档放到 examples/basic_edition/mqtt 目录下，覆盖 iot-product.json 文件。
4. 通过脚本自动生成演示配置文件。

```shell
# 进入工具脚本目录
cd tools
python tc_iot_code_generator.py -c ../examples/basic_edition/coap/iot-product.json code_templates/tc_iot_device_config.h
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/basic_edition/coap/iot-product.json 文件成功
文件 ../examples/basic_edition/coap/tc_iot_device_config.h 生成成功
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
编译完成后，在 build/bin/ 目录下，会产生一个 basic_coap 程序。

```shell
# 开启详细日志模式，运行 basic_coap 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_coap_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s secret_abc  认证模式为Token模式时，-s 指定Device Secret
# 如果已经在 tc_iot_coap_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./basic_coap --trace -p 5683

./basic_coap -d device_xxxx -s secret_abc --trace -p 5683

# 如运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./basic_coap -d device_xxxx -p 5683

```

## 数据交互流程
- 下图展示的流程为：设备定时上报数据的流程。

![图例](https://user-images.githubusercontent.com/990858/44085055-05d1da92-9feb-11e8-9221-163bc9a5fe23.png)

## SDK API 样例及说明

### 1. 初始化
tc_iot_coap_construct 初始化 CoAP 客户端数据。

#### 样例

```c
    tc_iot_coap_construct(p_coap_client, p_coap_config);
```

#### 函数原型及说明

```c
/**
 * @brief tc_iot_coap_construct 初始化 CoAP 客户端数据
 *
 * @param c 待初始化的 CoAP 客户端数据结构。
 * @param p_client_config 初始化相关参数，包括
 * CoAP 服务地址及端口、是否使用 DTLS及DTLS PSK、
 * 产品信息、设备名称、设备密钥、回调函数等。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_coap_construct(tc_iot_coap_client* c, tc_iot_coap_client_config* p_client_config);
```

### 2. 认证获取 Token
tc_iot_coap_auth 发起认证，获取后续服务所需的设备 Token。

#### 样例

```c
    ret = tc_iot_coap_auth(p_coap_client);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("CoAP auth failed, ret=%d.\n", ret);
        return 0;
    }
```

#### 函数原型及说明

```c
/**
 * @brief tc_iot_coap_auth 发起认证，获取后续服务所需的设备 Token。
 *
 * @param c 已初始化好的 CoAP 客户端。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_coap_auth(tc_iot_coap_client* c);
```

### 3. 发起上报请求 
调用 tc_iot_coap_publish 发送请求，获得响应数据。

#### 样例

```c
    int temperature = 35;
    while (!stop) {
        if (temperature >= 40 ) {
            step = -1;
        } else if (temperature <= -10) {
            step = 1;
        }
        temperature += step;

        tc_iot_hal_snprintf(buffer,sizeof(buffer), "{\"temperature\":%d}", temperature);

        // 基于 CoAP 协议上报数据
        tc_iot_coap_publish(&coap_client, TC_IOT_COAP_SERVICE_PUBLISH_PATH, pub_topic_query_param, buffer, NULL);
        tc_iot_hal_printf("Publish yielding ...\n");
        tc_iot_coap_yield(&coap_client, TC_IOT_COAP_MESSAGE_ACK_TIMEOUT_MS);

        for (i = 20; i > 0; i--) {
            tc_iot_hal_printf("%d ...", i);
            tc_iot_hal_sleep_ms(1000);
        }
        tc_iot_hal_printf("\n");
    }
```

#### 函数原型及说明

```c
/**
 * @brief tc_iot_coap_publish 为基于 tc_iot_coap_send_message
 * 的上层逻辑封装，用来单向发送上报消息。
 *
 * @param c 已成功获取获取授权 Token 的 CoAP 客户端。
 * @param uri_path 上报接口 URI Path，当前固定填写
 * TC_IOT_COAP_SERVICE_PUBLISH_PATH。
 * @param topic_query_uri 上报消息发送目的 Topic
 * 参数，参数固定格式为：tp=Topic_Name，即如果上报消息到 TopicA，
 * 参数应增加 tp= 前缀后，填写为 “tp=TopicA”
 * @param msg 上报消息 Payload 。
 * @param callback 上报结果回调。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 *
 */
int tc_iot_coap_publish( tc_iot_coap_client * c, const char * uri_path, 
        const char * topic_query_uri, const char * msg, tc_iot_coap_con_handler callback);

/**
 *
 * @brief tc_iot_coap_yield  CoAP client 主循环，包含上行消息响应超时
 * 检测、服务器下行消息收取等操作。
 *
 * @param c CoAP client 对象
 * @param timeout_ms 等待时延，单位毫秒
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_coap_yield(tc_iot_coap_client * c, int timeout_ms);
```

