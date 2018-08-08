# SDK接口说明
以下是C SDK 提供的功能和对应 API，用于设备端编写业务逻辑，API 接口暂不支持多线程调用，请勿在多线程环境下，跨线程调用。 更加详细的接口功能说明请查看 [include/tc_iot_export.h](https://tencentyun.github.io/tencent-cloud-iotsuite-embedded-c/sdk/tc__iot__export_8h.html) 中的注释。

## 1. 日志接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1          | tc_iot_set_log_level | 设置打印的日志等级 |
| 2          | tc_iot_get_log_level | 返回日志输出的等级 |
| 3          | tc_iot_log_level_enabled | 判断当前等级日志是否打开 |



## 2. MQTT 接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_mqtt_client_construct | 构造 MQTT client，并连接MQ服务器 |
| 2 | tc_iot_mqtt_client_destroy | 关闭 MQTT client 连接，并销毁 MQTT client |
| 3 | tc_iot_mqtt_client_yield | MQTT Client 主循环，包含心跳维持、上行消息响应超时检测、服务器下行消息收取等操作。|
| 4 | tc_iot_mqtt_client_publish | 向指定的 Topic 发布消息 |
| 5 | tc_iot_mqtt_client_subscribe | 订阅指定 Topic 的消息 |
| 6 | tc_iot_mqtt_client_unsubscribe | 取消订阅已订阅的 Topic |
| 7 | tc_iot_mqtt_client_is_connected | 判断 MQTT client 目前是否已连接 |
| 8 | tc_iot_mqtt_client_disconnecd | 断开 MQTT client 与服务端的连接 |

## 3. 数据模板接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_server_init | 根据设备配置参数，初始化服务。 |
| 2 | tc_iot_server_loop | 服务任务主循环函数，接收服务推送及响应数据。 |
| 3 | tc_iot_server_destroy | 数据模板服务析构处理，释放资源。 |
| 4 | tc_iot_report_device_data | 上报设备数据模板参数最新数据状态，更新到服务端。|
| 5 | tc_iot_confirm_devcie_data | 根据设备控制端要求，发送设备数据模板参数控制指令，更新到服务端，推送给设备。 |

## 4. CoAP 接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_coap_construct | 根据 CoAP 设备配置参数，初始化服务。 |
| 2 | tc_iot_coap_destroy | CoAP 服务析构处理，释放资源。 |
| 3 | tc_iot_coap_auth | 发起认证，获取后续服务所需的设备 Token。 |
| 4 | tc_iot_coap_send_message | 向服务端发送 CoAP 消息。 |
| 5 | tc_iot_coap_yield | CoAP client 主循环，包含上行消息响应超时检测、服务器下行消息收取等操作。|
| 6 | tc_iot_coap_get_message_code | 获取 CoAP 消息请求或返回码。 |
| 7 | tc_iot_coap_get_message_payload | 获取 CoAP 消息的 Payload 内容。|

## 4. OTA 接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_ota_construct | 根据设备配置参数，初始化 OTA 服务。 |
| 2 | tc_iot_ota_destroy | OTA 服务析构处理，取消 OTA 消息订阅，释放资源。 |
| 3 | tc_iot_ota_report_firm | 上报设备系统信息，例如，硬件版本、MAC 地址、IMEI、固件版本、SDK 版本等。|
| 4 | tc_iot_ota_report_upgrade | OTA 升级执行过程中，上报固件下载及升级进度。 |
| 5 | tc_iot_ota_request_content_length | 向固件下载服务器，发起 HTTP HEAD 请求，获取固件长度。|
| 6 | tc_iot_ota_download | 根据指定的固件 URL 地址，下载固件 |

## 5. HTTP 接口
| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_http_mqapi_rpc | 通过 HTTP 接口 调用数据模板数据服务。|


