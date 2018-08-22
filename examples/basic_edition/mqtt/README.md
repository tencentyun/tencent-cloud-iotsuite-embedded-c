# 操作指南
## 准备工作
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md)； 
2. 创建产品，“数据协议”选择“自定义”，创建如下自定义Topic：
    - ${product_id}/${device_name}/cmd ，用于设备端接收指令。
    - ${product_id}/${device_name}/update ，用于设备端上报数据。

3. 进入【基本信息】，点击【导出】，导出 iot-xxxxx.json 文档，将 iot-xxxxx.json 文档放到 examples/basic_edition/mqtt 目录下，覆盖 iot-product.json 文件。
4. 通过脚本自动生成演示配置文件。

```shell
# 进入工具脚本目录
cd tools
python tc_iot_code_generator.py -c ../examples/basic_edition/mqtt/iot-product.json code_templates/tc_iot_device_config.h
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/basic_edition/mqtt/iot-product.json 文件成功
文件 ../examples/basic_edition/mqtt/tc_iot_device_config.h 生成成功
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
### MQTT 示例
编译完成后，在 build/bin/ 目录下，会产生一个 basic_mqtt 程序。

```shell
# 运行 basic_mqtt 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./basic_mqtt --trace -p 1883

./basic_mqtt -d device_xxxx -s secret_abc --trace -p 1883

# 如运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./basic_mqtt -d device_xxxx

```

#### MQTT 收发消息
收发 MQTT 消息，参见 demo_mqtt.c 中tc_iot_mqtt_client_publish(发送消息) & tc_iot_mqtt_client_subscribe(订阅 Topic) 。

## 数据交互流程
- 下图展示的流程为：
    1. 设备上线，并订阅下行消息 Topic ；
    2. 用户调用 API，通过发送 MQTT 消息到指定 Topic的方式，发送控制指令；
    3. 服务端推送消息给设备，设备收到服务端推送的消息，进行处理，并上报处理结果;
    4. 服务端收到设备上报结果后，根据用户在规则引擎配置的地址，转发数据到用户的服务器；

![图例](https://user-images.githubusercontent.com/990858/44084061-504185d0-9fe8-11e8-97a7-b6824530ae87.png)

## SDK MQTT API 样例及说明

### 1. 初始化 SDK
tc_iot_mqtt_client_construct 初始化 MQTT 客户端，并与云端 MQ 服务建立连接

#### 样例

```c
    int ret = 0;
    ret = tc_iot_mqtt_client_construct(p_client, p_client_config);
```

#### 函数原型及说明

```c
/**
* @brief tc_iot_mqtt_client_construct 构造 MQTT client，并连接MQ服务器
*
* @param p_mqtt_client MQTT client 对象，出参。
* @param p_client_config 用来初始化 MQTT Client 对象的配置信息, 入参。
*
* @return 结果返回码
* @see tc_iot_sys_code_e
*/
int tc_iot_mqtt_client_construct(tc_iot_mqtt_client* p_mqtt_client,
                                tc_iot_mqtt_client_config* p_client_config);
```

### 2. 订阅主题
tc_iot_mqtt_client_subscribe 订阅云端下行消息 Topic，接收云端下发数据

#### 样例 

```c
// 订阅消息回调
void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("[s->c] %s\n", (char*)message->payload);
}
   
    ...
    int ret = 0;
    const char * sub_topic = "iot-product_id/device_name_001/cmd";
    ret = tc_iot_mqtt_client_subscribe(p_client,
              sub_topic,
              TC_IOT_QOS1,
              _on_message_received, // 回调函数，接收发布到当前订阅主题的消息
              NULL //context 参数，在回调 _on_message_received 是，
                   // 会通过 md->context 透传
              );
```

#### 函数原型及说明

```c
/**
 * @brief tc_iot_mqtt_client_subscribe 订阅指定一个或多个 Topic 的消息
 *
 * @param p_mqtt_client MQTT client 对象
 * @param topic_filter 待订阅 Topic 名称
 * @param qos 本次订阅的 QOS 等级
 * @param msg_handler 订阅消息回调
 * @param context 订阅响应回调 context
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_subscribe(tc_iot_mqtt_client* p_mqtt_client,
                                 const char* topic_filter,
                                 tc_iot_mqtt_qos_e qos,
                                 message_handler msg_handler,
                                 void * context);

```

### 3. 取消订阅

tc_iot_mqtt_client_unsubscribe 取消订阅已订阅的 Topic

#### 样例
```c
    int ret = 0;
    const char * sub_topic = "iot-product_id/device_name_001/cmd";
    ret = tc_iot_mqtt_client_unsubscribe(p_client,
                                    sub_topic // 待取消订阅的 Topic 名称
                                    );
```

#### 函数原型及说明
```c
/**
 * @brief tc_iot_mqtt_client_unsubscribe 取消对某个或多个 Topic 订阅
 *
 * @param p_mqtt_client MQTT client 对象
 * @param topic_filter 待取消订阅 Topic 名称
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_unsubscribe(tc_iot_mqtt_client* p_mqtt_client,
                                   const char* topic_filter);
```

### 4. 发布消息

tc_iot_mqtt_client_publish 向指定的上行 Topic 发布消息，上报数据给服务端

#### 样例

```c
    int ret = 0;
    const char * action_get = "{\"method\":\"get\"}";
    tc_iot_mqtt_message pubmsg;
    const char * pub_topic = "iot-product_id/device_name_001/update";

    memset(&pubmsg, '\0', sizeof(pubmsg));
    pubmsg.payload = action_get;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;

    ret = tc_iot_mqtt_client_publish(p_client, pub_topic, &pubmsg);

```

#### 函数原型及说明
```c
/**
 * @brief tc_iot_mqtt_client_publish 向指定的 Topic 发布消息
 *
 * @param p_mqtt_client MQTT client 对象
 * @param topic Topic 名称
 * @param msg 待发送消息
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_publish(tc_iot_mqtt_client* p_mqtt_client,
                               const char* topic, tc_iot_mqtt_message* msg);
```

### 5. MQTT 主循环

tc_iot_mqtt_client_yield 主循环，包含心跳维持、上行消息响应超时检测、服务器下行消息收取等操作。

#### 样例
```c
    int timeout = 200; // 单位为毫秒
    while (!stop) {
        tc_iot_mqtt_client_yield(p_client, timeout);
    }

```

#### 函数原型及说明
```c
/**
 * @brief tc_iot_mqtt_client_yield MQTT client
 * 主循环，包含心跳维持、上行消息响应超时检测、服务器下行消息收取等操作。
 *
 * @param p_mqtt_client MQTT client 对象
 * @param timeout_ms 等待时延，单位毫秒
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_yield(tc_iot_mqtt_client* p_mqtt_client, int timeout_ms);
```

### 6.断开连接

tc_iot_mqtt_client_disconnect 断开设备端与服务端的连接。
#### 样例

```c
    tc_iot_mqtt_client_disconnect(p_client);
```

#### 函数原型及说明
```c
/**
 * @brief tc_iot_mqtt_client_disconnect 断开 MQTT client 与服务端的连接
 *
 * @param p_mqtt_client MQTT client 对象
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_mqtt_client_disconnect(tc_iot_mqtt_client* p_mqtt_client);
```

### 7. 释放资源
tc_iot_mqtt_client_destroy 释放相关资源。

#### 样例
```c
    tc_iot_mqtt_client_destroy(p_client);
```

#### 函数原型及说明
```c
/**
 * @brief tc_iot_mqtt_client_destroy 关闭 MQTT client 连接，并销毁 MQTT client
 *
 * @param p_mqtt_client MQTT client 对象
 */
void tc_iot_mqtt_client_destroy(tc_iot_mqtt_client* p_mqtt_client);
```
