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

3. 进入【基本信息】，点击【导出】，导出 iot-xxxxx.json 文档，将 iot-xxxxx.json 文档放到 examples/advanced_edition/coap 目录下，覆盖 iot-product.json 文件。
4. 通过脚本自动生成设备的逻辑框架及业务数据配置代码。

```shell
# 进入工具脚本目录
cd tools
python tc_iot_code_generator.py -c ../examples/advanced_edition/coap/iot-product.json code_templates/coap/*
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/advanced_edition/coap/iot-product.json 文件成功
文件 ../examples/advanced_edition/coap/coap_app_main.c 生成成功
文件 ../examples/advanced_edition/coap/tc_iot_device_config.h 生成成功
文件 ../examples/advanced_edition/coap/tc_iot_device_logic.h 生成成功

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
编译完成后，在 build/bin/ 目录下，会产生一个 advanced_coap 程序。

```shell
# 开启详细日志模式，运行 advanced_coap 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s secret_abc  认证模式为Token模式时，-s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./advanced_coap --trace -p 5683

./advanced_coap -d device_xxxx -s secret_abc --trace -p 5683

# 如运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./advanced_coap -d device_xxxx -p 5683

```

## 数据交互流程
- 下图展示的流程为：控制端下发指令；设备定时轮询，读取指令数据并上报及清除指令的流程。

![图例](https://user-images.githubusercontent.com/990858/44085090-18cbaaa6-9feb-11e8-9a6a-5ee23813480c.png)

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

### 3. 发起 RPC 请求 
调用 tc_iot_coap_rpc 发送请求，获得响应数据。

#### 样例

```c
    int ret = 0;
    char request[1024];
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    tc_iot_json_writer_open(w, request, sizeof(request));
    tc_iot_json_writer_string(w ,"method", "get");
    tc_iot_json_writer_bool(w ,"metadata", metadata);
    tc_iot_json_writer_bool(w ,"reported", reported);
    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    } else {
        tc_iot_hal_printf("[c->s]:%s\n", request);
    }

    // 基于 CoAP 协议的 RPC 调用
    ret = tc_iot_coap_rpc(p_coap_client, TC_IOT_COAP_SERVICE_RPC_PATH, rpc_pub_topic_query_param, 
                    rpc_sub_topic_query_param, request, _coap_con_get_rpc_handler);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("request failed ,ret=%d", ret);
    }
    ret = tc_iot_coap_yield(p_coap_client, TC_IOT_COAP_MESSAGE_ACK_TIMEOUT_MS);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("coap yield failed ,ret=%d", ret);
    }
    return ret;
```

#### 函数原型及说明

```c
/**
 * @brief tc_iot_coap_rpc 为基于 tc_iot_coap_send_message 
 * 的上层逻辑封装，用来调用影子服务或基于自定义 Topic 的远程服务。
 *
 * @param c 已成功获取获取授权 Token 的 CoAP 客户端。
 * @param uri_path 上报接口 URI Path，当前固定填写
 * TC_IOT_COAP_SERVICE_RPC_PATH。
 * @param topic_query_uri RPC 请求参数发送目的 Topic
 * 参数，参数固定格式为：pt=Topic_Name，即如果到 TopicUpdate，
 * 参数应增加 pt= 前缀后，填写为 “pt=TopicUpdate”
 * @param topic_resp_uri RPC 调用响应 Topic
 * 参数，参数固定格式为：st=Topic_Name，即如果到 TopicCmd，
 * 参数应增加 st= 前缀后，填写为 “st=TopicUpdate”
 * @param msg 上报消息 Payload 。
 * @param callback RPC 调用结果回调。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 *
 */
int tc_iot_coap_rpc( tc_iot_coap_client * c, const char * uri_path, 
        const char * topic_query_uri, const char * topic_resp_uri,
        const char * msg, tc_iot_coap_con_handler callback);

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

### 4. 析构资源
 tc_iot_coap_destroy 释放 CoAP client 对象相关资源。

#### 样例

```c
    tc_iot_coap_destroy(p_coap_client);
```

#### 函数原型及说明

```c
/**
 * @brief tc_iot_coap_destroy 释放 CoAP client 对象相关资源。
 *
 * @param c CoAP client 对象
 */
void tc_iot_coap_destroy(tc_iot_coap_client* c);
```
