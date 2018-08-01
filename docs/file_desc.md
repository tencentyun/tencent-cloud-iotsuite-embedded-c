
# 源码目录接口及文件作用说明

```shell
include # 头文件根目录
├── coap # CoAP Client及协议相关
│   └── tc_iot_coap.h # CoAP 协议头文件
├── common # 通用基础库
│   ├── tc_iot_base64.h # base64 编解码
│   ├── tc_iot_bit.h # bit 位操作相关宏及函数
│   ├── tc_iot_certs.h # 内置 HTTPS 和 MQTT over TLS CA 证书
│   ├── tc_iot_const.h # 错误码及通用结构、函数定义
│   ├── tc_iot_hmac.h  # sha256 算法头文件
│   ├── tc_iot_http_utils.h # HTTP 协议打包解包定义
│   ├── tc_iot_json.h # JSON 解析定义
│   ├── tc_iot_log.h # 日志函数定义
│   ├── tc_iot_md5.h # md5 哈希算法头文件
│   ├── tc_iot_safety_check.h # Assert 宏定义
│   ├── tc_iot_sign_utils.h # HTTPS接口及MQTT连接鉴权签名逻辑
│   ├── tc_iot_string_utils.h # 二进制与HEX格式字符串互转
│   ├── tc_iot_url.h # URL 解析及编解码
│   └── tc_iot_ya_buffer.h # Buffer 操作
├── http # HTTP 请求接口封装
│   └── tc_iot_token.h # 动态令牌接口及设备激活接口定义
├── mqtt # MQTT 接口
│   ├── tc_iot_client.h # MQTT Client 定义
│   └── tc_iot_mqtt.h # MQTT 协议定义
├── iotsuite # IoT Suite 相关接口
│   └── tc_iot_shadow.h # 影子及数据模板定义
├── ota # OTA 接口
│   ├── tc_iot_ota_download.h # OTA 固件下载定义
│   └── tc_iot_ota_mqtt.h # OTA 指令收发定义
├── platform # 平台适配，移植接口
│   ├── generic # 通用基础模板
│   │   └── tc_iot_platform.h # 平台相关定义
│   ├── linux # Linux 平台定义
│   │   └── tc_iot_platform.h # Linux 平台接口定义及头文件包含 
│   ├── tc_iot_hal_network.h # Network 通用网络结构及接口申明
│   ├── tc_iot_hal_os.h # 操作系统相关：时间戳、内存、字符串格式化、随机数等。
│   └── tc_iot_hal_timer.h # 定时器
├── tc_iot_compile_flags.h.in # 编译宏定义文件，本文件会被 cmake 引用并用来生成 tc_iot_compile_flags.h 。
├── tc_iot_compile_flags.h # 基于 cmake 编译参数生成的编译宏定义文件。
├── tc_iot_config.h # 基础配置宏定义
├── tc_iot_export.h # 对外提供的 API 接口，引用 SDK 时，只需要直接包含本文件即可，其他头文件都会通过本文件，间接包含引入。
├── tc_iot_external.h # 引用的第三方外部库 Paho MQTT, JSMN 等外部头文件
└── tc_iot_inc.h # 内部总体包含头文件

src # 源码根目录
├── CMakeLists.txt 
├── coap # CoAP 源码
│   ├── CMakeLists.txt
│   ├── tc_iot_coap.c # CoAP 实现
│   └── tc_iot_coap_code_map.c # CoAP 返回码及错误码消息映射
├── common # 通用基础库实现
│   ├── CMakeLists.txt
│   ├── tc_iot_base64.c # base64 编解码
│   ├── tc_iot_certs.c # 内置根证书
│   ├── tc_iot_hmac.c # sha256 算法
│   ├── tc_iot_http_utils.c # http 请求解析
│   ├── tc_iot_json.c # JSON 解析
│   ├── tc_iot_log.c # 日志实现
│   ├── tc_iot_md5.c # md5 算法
│   ├── tc_iot_sign_utils.c # HTTPS接口及MQTT连接鉴权签名逻辑
│   ├── tc_iot_string_utils.c # # 二进制与HEX格式字符串互转
│   ├── tc_iot_url.c # URL 解析及编解码
│   └── tc_iot_ya_buffer.c # Buffer 实现
├── http # HTTP 请求接口封装
│   ├── CMakeLists.txt
│   ├── tc_iot_active_device.c # 设备动态激活接口调用逻辑
│   ├── tc_iot_http_mqapi.c # HTTP rpc 接口调用逻辑 
│   └── tc_iot_token.c # HTTP 动态令牌获取
├── iotsuite # Iot Suite 实现
│   ├── CMakeLists.txt
│   ├── tc_iot_shadow.c # 影子服务
│   └── tc_iot_shadow_property.c # 数据模板
├── mqtt # MQTT 接口实现
│   ├── CMakeLists.txt
│   ├── tc_iot_client.c # MQTT Client 实现
│   └── tc_iot_mqtt.c# MQTT 协议处理
├── ota # OTA 实现
│   ├── CMakeLists.txt
│   ├── tc_iot_ota_download.c # OTA 下载逻辑
│   └── tc_iot_ota_mqtt.c # OTA 指令收发
├──external #  依赖的外部第三方库
│    ├── jsmn # JSON 解析
│    ├── mbedtls # TLS & DTLS 实现
│    └── paho.mqtt.embedded-c # MQTT 打包及解包
└── platform # 不同平台移植实现
    ├── CMakeLists.txt
    ├── generic # 略
    └── linux # Linux 平台移植实现
        ├── CMakeLists.txt 
		├── tc_iot_hal_util.c # 工具函数
        ├── tc_iot_hal_os.c # 时钟、内存、随机数等接口实现
        ├── tc_iot_hal_timer.c # 定时器实现
        ├── tc_iot_hal_net.c # TCP 实现
        ├── tc_iot_hal_tls.c # TLS 实现
        ├── tc_iot_hal_dtls.c # DTLS 实现
        └── tc_iot_hal_udp.c # UDP接口实现

```

