# SDK接口说明
以下是 v1.0.0 版本 C SDK 提供的功能和对应 API，用于客户编写业务逻辑，更加详细的说明请查看 include/tc_iot_export.h 中的注释。

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

3. 设备影子接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_shadow_construct | 构造 shadow client 对象 |
| 2 | tc_iot_shadow_destroy | 关闭 shadow client 连接，并销毁 shadow client |
| 3 | tc_iot_shadow_yield | 在当前线程为底层 shadow client让出一定 CPU 执行时间 |
| 4 | tc_iot_shadow_update | 异步更新设备影子文档 |
| 5 | tc_iot_shadow_get | 异步方式获取设备影子文档 |
| 6 | tc_iot_shadow_delete | 异步方式删除设备影子文档 |



