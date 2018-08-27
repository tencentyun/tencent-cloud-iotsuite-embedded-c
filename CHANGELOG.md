# 变更记录

## [2.8] - 2018-08-27
### 新增
- 支持数据模板版本 sequence 属性回传。

## [2.7] - 2018-08-16
### 新增
- MQTT 支持签名认证模式。
- 数据模板布尔类型兼容0/1。

### 变更
- 示例程序结构及文档优化重构。
- yield 收包时延顺延优化。
- bug fix 。

## [2.6] - 2018-07-12
### 新增
- 支持 HTTP RPC 功能，可使用 HTTP 协议与物联网套件数据模板云端交互，示例详见 examples/linux/http 。

### 变更
- 目录组织结构，依据细化功能划分 MQTT(mqtt)、数据模板(iotsuite)、CoAP(coap)、OTA(ota)、HTTP Token&RPC(http)等。
- 编译配置方式调整：
    1. 内存及主要参数配置放在 include/tc_iot_config.h 。
    2. 编译宏放在 include/tc_iot_compile_flags.h 。
        > cmake 会根据命令行和 CMakeLists.txt 参数设置，用 include/tc_iot_compile_flags.h.in 
        >生成 include/tc_iot_compile_flags.h，移植到其他平台时，可跳过 
        >include/tc_iot_compile_flags.h.in，直接拷贝及修改 
        >include/tc_iot_compile_flags.h 即可。
- 数据模板 get 指令默认设定不返回 metadata，精简 get 指令的 reply 响应包。
- bug fix。

## [2.5] - 2018-06-15
### 新增
- 支持 OTA 功能。

## [2.3] - 2018-05-22
### 新增
- 数据模板功能，支持字符串类型数据。
- 支持 CoAP 协议。

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

