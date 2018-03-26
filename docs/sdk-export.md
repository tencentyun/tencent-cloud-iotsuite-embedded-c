# SDK接口说明
以下是 v1.0.0 版本 C SDK 提供的功能和对应 API，用于客户编写业务逻辑，更加详细的说明请查看 [include/tc_iot_export.h](https://tencentyun.github.io/tencent-cloud-iotsuite-embedded-c/sdk/tc__iot__export_8h.html) 中的注释。

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
| 3 | tc_iot_mqtt_client_yield | 在当前线程为底层 MQTT client，让出一定 CPU 执行时间 |
| 4 | tc_iot_mqtt_client_publish | 向指定的 Topic 发布消息 |
| 5 | tc_iot_mqtt_client_subscribe | 订阅指定 Topic 的消息 |
| 6 | tc_iot_mqtt_client_unsubscribe | 取消订阅已订阅的 Topic |
| 7 | tc_iot_mqtt_client_is_connected | 判断 MQTT client 目前是否已连接 |
| 8 | tc_iot_mqtt_client_disconnecd | 断开 MQTT client 与服务端的连接 |

3. 数据点接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_server_init | 根据设备配置参数，初始化服务。 |
| 2 | tc_iot_server_loop | 服务任务主循环函数，接收服务推送及响应数据。 |
| 3 | tc_iot_server_destroy | 数据点服务析构处理，释放资源。 |
| 4 | tc_iot_report_propeties | 上报设备数据点参数最新数据状态，更新到服务端。|
| 5 | tc_iot_set_control_propeties | 根据设备控制端要求，发送设备数据点参数控制指令，更新到服务端，推送给设备。 |
| 6 | tc_iot_report_firm | 上报设备系统信息，例如，硬件版本、MAC 地址、IMEI、固件版本、SDK 版本等。|


