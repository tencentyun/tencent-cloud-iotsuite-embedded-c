# 变更记录

## [2.0.0] - 2018-04-18
### 新增
- 新增数据模板功能，支持数据模板变更回调及数据自动上报逻辑。
- tools 目录集成 tc_iot_code_generator.py 命令行工具，用来为配置的数据模板生成代码。

### 变更
- bug fix 。

## [1.0.6] - 2018-03-01
### 新增
- 设备影子 get、update、delete 接口，支持单个请求独立指定回调，用来定向回调，
以及判定设备影子请求是否超时。
- example 新增 light 控制模式 demo 。
- tools 目录集成 tc_iot_shadow_cli.py 命令行工具，用来通过 API 接口，读取或更新影子设
备数据。

### 变更
- tc_iot_mqtt_client_subscribe 接口新增 session_context 字段，用来做回调回传
，如果不需要回传，session_context 字段传 NULL 即可。
- tc_iot_shadow_get, tc_iot_shadow_update, tc_iot_shadow_delete 接口调用方式变更
- samples 目录变更为 examples
- bug fix 。


## [1.0.5] - 2018-02-01
### 新增
- SDK 默认内置 token 服务官网和 MQ 服务根证书
- 使用自定义 Topic 进行通讯的场景 demo
- SDK函数及结构定义注释说明文档上线
    https://tencentyun.github.io/tencent-cloud-iotsuite-embedded-c/sdk/index.html

### 变更
- 日志优化及缺陷修复
- samples 目录结构调整


## [1.0.0] - 2018-01-23
### 基础功能发布
- 支持 MQTT 直连模式访问云端。
- 支持设备影子上报及访问。
- 支持通过 Token 服务动态认证方式连接云端。
- 支持 MQTT over TLS 访问云端。
- 支持 HTTPS 访问 Token 服务。

