# 操作指南
## 准备工作 & 编译本示例
1. 参见 [开发准备](https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/blob/master/README.md)； 
2. 配置固件版本号 tc_iot_ota_logic.h ：
```shell
#define TC_IOT_FIRM_VERSION "LINUXV1.0"
```
3. 进入【基本信息】，点击【导出】，导出 iot-xxxxx.json 文档，将 iot-xxxxx.json 文档放到 examples/scenarios/ota 目录下，覆盖 iot-product.json 文件。
4. 通过脚本自动生成演示配置文件。

```shell
# 进入工具脚本目录
cd tools
python tc_iot_code_generator.py -c ../examples/scenarios/ota/iot-product.json code_templates/tc_iot_device_config.h
```

执行成功后会看到有如下提示信息：
```shell
加载 ../examples/scenarios/ota/iot-product.json 文件成功
文件 ../examples/scenarios/ota/tc_iot_device_config.h 生成成功
```

5. 修改 tc_iot_device_config.h 配置，设置 Device Name 和 Device Secret：
```c
/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"
```

6. 代码及配置生成成功后，进入 build 目录，开始编译。

```shell
cd ../build
make
```

## 运行程序
编译完成后，在 build/bin/ 目录下，会产生一个 scn_ota 程序。

```shell
# 运行 scn_ota 设备端应用，
# 此种方式运行，可以有效查看日志及抓包定位问题
# 备注：
# -d device_xxxx 参数是指定当前连接使用的设备名
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_NAME 指定了
# 正确的设备名称，则命令行执行时，可以不用指定 -d device_xxx 参数。
#
# -s 指定Device Secret
# 如果已经在 tc_iot_device_config.h 中，为TC_IOT_CONFIG_DEVICE_SECRET 指定了
# 正确的Device Secret，则命令行执行时，可以不用指定 -s secret_abc 参数。
# ./scn_ota --trace -p 1883

./scn_ota -d device_xxxx -s secret_abc --trace -p 1883

# 如 scn_ota 运行正常未见异常
# 也可用默认模式来执行，避免日志干扰
./scn_ota -d device_xxxx

```

## 控制台操作
控制台上传固件，配置升级任务，下发给当前设备，即可体验 OTA 流程。

## 数据交互流程
![图例](https://user-images.githubusercontent.com/990858/44129418-c946d2f4-a07a-11e8-8867-9f379303f802.png)

## OTA 功能开发指引
1. 初始化 OTA 服务：
```c
tc_iot_ota_handler handler;
...
    // 订阅 OTA 下行 Topic，格式为： /ota/get/${product_id}/${device_name}
    tc_iot_hal_snprintf(ota_sub_topic, sizeof(ota_sub_topic), "ota/get/%s/%s", 
            p_client_config->device_info.product_id, 
            p_client_config->device_info.device_name);

    // 配置 OTA 上行 Topic，格式为： /ota/update/${product_id}/${device_name}
    tc_iot_hal_snprintf(ota_pub_topic, sizeof(ota_pub_topic), "ota/update/%s/%s", 
            p_client_config->device_info.product_id, 
            p_client_config->device_info.device_name);

    // 初始化 OTA 服务
    ret = tc_iot_ota_construct(ota_handler, p_client, ota_sub_topic, ota_pub_topic, _on_ota_message_received);

    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("init ota handler failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
    }

```

2. 上报固件版本等信息：
```c
    // 上报设备信息及当前版本号
    tc_iot_ota_report_firm(&handler,
            "sdk-ver", TC_IOT_SDK_VERSION,  // 上报 SDK 版本
            "firm-ver",TC_IOT_FIRM_VERSION,  // 上报固件信息，OTA 升级版本号判断依据
            NULL); // 最后一个参数固定填写 NULL，作为变参结束判断

```
3. 收到升级指令后，开始升级流程：
```c
    ...
    // 上报升级指令已收到
    tc_iot_ota_report_upgrade(ota_handler, OTA_COMMAND_RECEIVED, NULL, 0);
    ...
    if (tc_iot_ota_version_larger(ota_handler->version, TC_IOT_FIRM_VERSION)) {
        // 上报版本检查结果
        tc_iot_ota_report_upgrade(ota_handler, OTA_VERSION_CHECK, TC_IOT_OTA_MESSAGE_SUCCESS, 0);

        // 上报下载进度，最后一个参数表示当前下载进度百分比
        tc_iot_ota_report_upgrade(ota_handler, OTA_DOWNLOAD, NULL, 0);
        
        // 开始下载固件
        do_download(ota_handler->download_url, ota_handler->version, ota_handler->firmware_md5);
    }
    ...
    // 请求固件下载服务器
    ret = tc_iot_ota_download(download_url, partial_start, my_http_download_callback, &helper);

    ...
    // 上报下载进度
    tc_iot_ota_report_upgrade(helper->ota_handler, OTA_DOWNLOAD, NULL, new_percent);

    ...
    // 上报 MD5SUM 检查结果
    tc_iot_ota_report_upgrade(ota_handler, OTA_MD5_CHECK, "success", 0);

    // 开始升级，例如，覆盖旧的固件区域或设置启动参数，切换启动分区
    // 升级或切换完成后，上报开始升级。
    tc_iot_ota_report_upgrade(ota_handler, OTA_UPGRADING, NULL, 0);
```

4. 固件下载及升级操作完成后，重启设备，启用新固件。设备启动后，通过 OTA
   服务上报最新版本，OTA 服务根据上报的最新版本，确认升级任务成功。

