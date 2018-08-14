##  开发准备

### SDK 获取

腾讯云 IoT Suite C SDK 的下载地址： [tencent-cloud-iotsuite-embedded-c.git](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c.git)

```shell
git clone https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c.git
```


### 开发环境
1. SDK 在 Linux 环境下的测试和验证，主要基于 Ubuntu 16.04 版本，gcc-5.4 (建议至少 gcc-4.7+)，Python 2.7.12+(代码生成及控制台命令行脚本)，cmake 2.8+。

```shell
sudo apt install cmake python2.7 git build-essential
```

2. 配置并运行示例：
- [基础版 MQTT 示例](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/examples/basic_edition/mqtt)
- [高级版 MQTT 示例](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/examples/advanced_edition/mqtt)

### 编译及运行
1. 执行下面的命令，编译示例程序：

```shell
cd tencent-cloud-iotsuite-embedded-c
mkdir -p build
cd build
cmake ../
make
```

2. 编译后，build目录下的关键输出及说明如下：

```shell
bin
|-- basic_mqtt               # 基础版 MQTT 连接云服务演示程序
|-- basic_coap               # 基础版 CoAP 连接云服务演示程序
|-- advaned_mqtt             # 高级版 MQTT 连接云服务演示程序
|-- advaned_coap             # 高级版 CoAP 连接云服务演示程序
|-- scn_ota                  # OTA 功能演示程序
|-- scn_smartbox             # 基于基础版 MQTT 协议开发的智能货柜演示程序
|-- scn_light                # 基于高级版 MQTT 协议开发的智能灯演示程序
lib
|-- libtc_iot_common.a      # SDK 基础工具库，负责http、json、base64等解析和编解码功能
|-- libtc_iot_hal.a         # SDK 的硬件及操作系统抽象，负责内存、定时器、网络交互等功能
|-- libtc_iot_mqtt_client.a # SDK 基础版 MQTT 协议库，负责 MQTT 协议解析及连接管理
|-- libtc_iot_suite.a       # SDK 高级版功能库，基于 MQTT 封装了高级版数据模板功能。
|-- libtc_iot_coap.a        # SDK CoAP 协议封装，用于资源受限设备，通过 CoAP 协议使用基础版消息上行及高级版数据模板功能。
|-- libtc_iot_http_mqapi.a  # SDK HTTP RPC 协议封装，用于资源受限设备，通过 HTTP 协议使用高级版数据模板功能。
|-- libtc_iot_ota.a         # SDK OTA 功能库，提供了 OTA 协议收发处理及固件下载功能
|-- libMQTTPacketClient.a   # 第三方库，用于 MQTT 协议解析
|-- libmbedtls.a            # 第三方库，用于 TLS 及 DTLS 协议处理
|-- libjsmn.a               # 第三方库，用于 JSON 协议解析

```

3. 执行示例程序：

```shell
# 注意：在运行前，每个示例程序都需要预先配置对应的产品 ID、设备名称、设备密钥等信息，
# 请注意参考 examples 目录下对应示例程序的 README，进行预先配置。

cd bin

# 运行demo程序
./basic_mqtt
# or
./advanced_app

...

```

## SDK接口说明
以下是C SDK 提供的功能和对应 API，用于设备端编写业务逻辑，API 接口暂不支持多线程调用，在多线程环境下，请勿跨线程调用。 更加详细的接口功能说明请查看 [include/tc_iot_export.h](include/tc_iot_export.h) 中的注释。

### 1. 日志接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1          | tc_iot_set_log_level | 设置打印的日志等级 |
| 2          | tc_iot_get_log_level | 返回日志输出的等级 |
| 3          | tc_iot_log_level_enabled | 判断当前等级日志是否打开 |

### 2. MQTT 鉴权接口
| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1          | tc_iot_refresh_auth_token | 鉴权模式为动态令牌模式时，通过本接口获取访问 MQTT 服务端动态用户名和密码|

### 3. MQTT 接口

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

### 4. 数据模板接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_server_init | 根据设备配置参数，初始化服务。 |
| 2 | tc_iot_server_loop | 服务任务主循环函数，接收服务推送及响应数据。 |
| 3 | tc_iot_server_destroy | 数据模板服务析构处理，释放资源。 |
| 4 | tc_iot_report_device_data | 上报设备数据模板参数最新数据状态，更新到服务端。|
| 5 | tc_iot_confirm_devcie_data | 根据设备控制端要求，发送设备数据模板参数控制指令，更新到服务端，推送给设备。 |

### 5. CoAP 接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_coap_construct | 根据 CoAP 设备配置参数，初始化服务。 |
| 2 | tc_iot_coap_destroy | CoAP 服务析构处理，释放资源。 |
| 3 | tc_iot_coap_auth | 发起认证，获取后续服务所需的设备 Token。 |
| 4 | tc_iot_coap_send_message | 向服务端发送 CoAP 消息。 |
| 5 | tc_iot_coap_yield | CoAP client 主循环，包含上行消息响应超时检测、服务器下行消息收取等操作。|
| 6 | tc_iot_coap_get_message_code | 获取 CoAP 消息请求或返回码。 |
| 7 | tc_iot_coap_get_message_payload | 获取 CoAP 消息的 Payload 内容。|

### 6. OTA 接口

| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_ota_construct | 根据设备配置参数，初始化 OTA 服务。 |
| 2 | tc_iot_ota_destroy | OTA 服务析构处理，取消 OTA 消息订阅，释放资源。 |
| 3 | tc_iot_ota_report_firm | 上报设备系统信息，例如，硬件版本、MAC 地址、IMEI、固件版本、SDK 版本等。|
| 4 | tc_iot_ota_report_upgrade | OTA 升级执行过程中，上报固件下载及升级进度。 |
| 5 | tc_iot_ota_request_content_length | 向固件下载服务器，发起 HTTP HEAD 请求，获取固件长度。|
| 6 | tc_iot_ota_download | 根据指定的固件 URL 地址，下载固件 |

### 7. HTTP 接口
| 序号        | 函数名      | 说明        |
| ---------- | ---------- | ---------- |
| 1 | tc_iot_http_mqapi_rpc | 通过 HTTP 接口 调用数据模板数据服务。|



## 移植说明
### 硬件及操作系统平台抽象层（HAL 层）
SDK 抽象定义了硬件及操作系统平台抽象层（HAL 层），将所依赖的内存、定时器、网络传输交互等功能，
都封装在 HAL 层（对应库libtc_iot_hal）中，进行跨平台移植时，首先都需要根据对应平台的硬件及操作系统情况，
对应适配或实现相关的功能。

平台移植相关的头文件及源文件代码结构如下：
```shell
include/platform/
|-- linux                   # 不同的平台或系统，单独建立独立的目录
|   |-- tc_iot_platform.h   # 引入对应平台相关的定义或系统头文件
|-- tc_iot_hal_network.h    # 网络相关定义
|-- tc_iot_hal_os.h         # 操作系统内存、时间戳等相关定义
|-- tc_iot_hal_timer.h      # 定时器相关定义
src/platform/
|-- CMakeLists.txt
|-- linux
    |-- CMakeLists.txt
    |-- tc_iot_hal_net.c    # TCP 非加密直连方式网络接口实现
    |-- tc_iot_hal_os.c     # 内存及时间戳实现
    |-- tc_iot_hal_timer.c  # 定时器相关实现
    |-- tc_iot_hal_tls.c    # TLS 加密网络接口实现
    |-- tc_iot_hal_udp.c    # UDP 接口实现
    |-- tc_iot_hal_dtls.c    # DTLS 加密网络接口实现
```

C-SDK 中提供的 HAL 层是基于 Linux 等 POSIX 体系系统的参考实现，但并不强耦合要求实现按照 POSIX 接口方式，移植时可根据目标系统的情况，灵活调整。

所有 HAL 层函数都在 include/platform/tc_iot_hal*.h 中进行声明，函数都以 tc_iot_hal为前缀。

以下是需要实现的 HAL 层接口，详细信息可以参考注释。

### 基础功能
| 功能分类    | 函数名     | 说明        | 是否可选   |
| ---------- | ---------- | ---------- | ---------- |
| 内存 | tc_iot_hal_malloc | 分配所需的内存空间，并返回一个指向它的指针。 | 基础必选 |
| 内存 | tc_iot_hal_free | 释放之前调用 tc_iot_hal_malloc 所分配的内存空间。 | 基础必选 |
| 输入输出 | tc_iot_hal_printf | 发送格式化输出到标准输出 stdout。 | 基础必选 |
| 输入输出 | tc_iot_hal_snprintf | 发送格式化输出到字符串。 | 基础必选 |
| 时间日期 | tc_iot_hal_timestamp | 系统时间戳，格林威治时间 1970-1-1 00点起总秒数 | 基础必选 |
| 定时器 | tc_iot_hal_sleep_ms | 睡眠挂起一定时长，单位：ms | 基础必选 |
| 定时器 | tc_iot_hal_timer_init | 初始化或重置定时器 | 基础必选 |
| 定时器 | tc_iot_hal_timer_is_expired | 判断定时器是否已经过期 | 基础必选 |
| 定时器 | tc_iot_hal_timer_countdown_ms | 设定定时器时延，单位：ms | 基础必选 |
| 定时器 | tc_iot_hal_timer_countdown_second | 设定定时器时延，单位：s | 基础必选 |
| 定时器 | tc_iot_hal_timer_left_ms | 检查定时器剩余时长，单位：ms | 基础必选 |
| 随机数 | tc_iot_hal_srandom | 设置随机数种子值 | 基础必选 |
| 随机数 | tc_iot_hal_random | 获取随机数 | 基础必选 |

### 网络功能（二选一或全选）
根据实际连接方式选择，如是否走MQTT over TLS加密，是否通过HTTPS接口获取Token等，选择性实现 TCP 或 TLS 相关接口。

#### 基于 MQTT 协议接入
##### TCP

| 功能分类    | 函数名     | 说明        | 是否可选   |
| ---------- | ---------- | ---------- | ---------- |
| TCP 连接 | tc_iot_hal_net_init | 初始化网络连接数据 | 可选，非加密直连时实现 |
| TCP 连接 | tc_iot_hal_net_connect | 连接服务端 | 可选，非加密直连时实现 |
| TCP 连接 | tc_iot_hal_net_is_connected | 判断当前是否已成功建立网络连接 | 可选，非加密直连时实现 |
| TCP 连接 | tc_iot_hal_net_write | 发送数据到网络对端 | 可选，非加密直连时实现 |
| TCP 连接 | tc_iot_hal_net_read | 接收网络对端发送的数据 | 可选，非加密直连时实现 |
| TCP 连接 | tc_iot_hal_net_disconnect | 断开网络连接 | 可选，非加密直连时实现 |
| TCP 连接 | tc_iot_hal_net_destroy | 释放网络相关资源 | 可选，非加密直连时实现 |

##### TLS

| 功能分类    | 函数名     | 说明        | 是否可选   |
| ---------- | ---------- | ---------- | ---------- |
| TLS 连接 | tc_iot_hal_tls_init | 初始化 TLS 连接数据 | 可选，基于TLS加密通讯时实现 |
| TLS 连接 | tc_iot_hal_tls_connect | 连接 TLS 服务端并进行相关握手及认证 | 可选，基于TLS加密通讯时实现 |
| TLS 连接 | tc_iot_hal_tls_is_connected | 判断当前是否已成功建立 TLS 连接 | 可选，基于TLS加密通讯时实现 |
| TLS 连接 | tc_iot_hal_tls_write | 发送数据到 TLS 对端 | 可选，基于TLS加密通讯时实现 |
| TLS 连接 | tc_iot_hal_tls_read | 接收 TLS 对端发送的数据 | 可选，基于TLS加密通讯时实现 |
| TLS 连接 | tc_iot_hal_tls_disconnect | 断开 TLS 连接 | 可选，基于TLS加密通讯时实现 |
| TLS 连接 | tc_iot_hal_tls_destroy | 释放 TLS 相关资源 | 可选，基于TLS加密通讯时实现 |

#### 基于 CoAP 协议接入
##### UDP

| 功能分类    | 函数名     | 说明        | 是否可选   |
| ---------- | ---------- | ---------- | ---------- |
| UDP 连接 | tc_iot_hal_udp_init | 初始化网络连接数据 | 可选，非加密直连时实现 |
| UDP 连接 | tc_iot_hal_udp_connect | 连接服务端 | 可选，非加密直连时实现 |
| UDP 连接 | tc_iot_hal_udp_is_connected | 判断当前是否已成功建立网络连接 | 可选，非加密直连时实现 |
| UDP 连接 | tc_iot_hal_udp_write | 发送数据到网络对端 | 可选，非加密直连时实现 |
| UDP 连接 | tc_iot_hal_udp_read | 接收网络对端发送的数据 | 可选，非加密直连时实现 |
| UDP 连接 | tc_iot_hal_udp_disconnect | 断开网络连接 | 可选，非加密直连时实现 |
| UDP 连接 | tc_iot_hal_udp_destroy | 释放网络相关资源 | 可选，非加密直连时实现 |

##### DTLS

| 功能分类    | 函数名     | 说明        | 是否可选   |
| ---------- | ---------- | ---------- | ---------- |
| DTLS 连接 | tc_iot_hal_dtls_init | 初始化 DTLS 连接数据 | 可选，基于DTLS加密通讯时实现 |
| DTLS 连接 | tc_iot_hal_dtls_connect | 连接 DTLS 服务端并进行相关握手及认证 | 可选，基于DTLS加密通讯时实现 |
| DTLS 连接 | tc_iot_hal_dtls_is_connected | 判断当前是否已成功建立 DTLS 连接 | 可选，基于DTLS加密通讯时实现 |
| DTLS 连接 | tc_iot_hal_dtls_write | 发送数据到 DTLS 对端 | 可选，基于DTLS加密通讯时实现 |
| DTLS 连接 | tc_iot_hal_dtls_read | 接收 DTLS 对端发送的数据 | 可选，基于DTLS加密通讯时实现 |
| DTLS 连接 | tc_iot_hal_dtls_disconnect | 断开 DTLS 连接 | 可选，基于DTLS加密通讯时实现 |
| DTLS 连接 | tc_iot_hal_dtls_destroy | 释放 DTLS 相关资源 | 可选，基于DTLS加密通讯时实现 |

