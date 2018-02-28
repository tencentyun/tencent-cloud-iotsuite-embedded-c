#!/bin/bash

# 获取调用接口的密钥信息：
# https://console.qcloud.com/capi
SECRET_ID=${TENCENT_CLOUD_SECRET_ID}
SECRET_KEY=${TENCENT_CLOUD_SECRET_KEY}

# 获取产品ID和设备名称信息：
# https://console.qcloud.com/iotsuite/product
prodcut_id=iot-fdxt0fie
device_name=light001


# light shadow 影子属性格式：
# {"name":"colorful light","color":16777215,"brightness":100,"light_switch":false}

cmd_cfg="--product_id=$prodcut_id --device_name=$device_name -u ${SECRET_ID} -p ${SECRET_KEY}"

# 通过控制台接口删除设备影子属性数据
# ./bin/tc_iot_shadow_cli.py UpdateIotShadow $cmd_cfg --shadow='{"desired":null,"reported":null}' 

# 通过控制台接口更新设备影子属性数据
./bin/tc_iot_shadow_cli.py UpdateIotShadow $cmd_cfg --shadow='{"desired":{"name":"light abc","color":256,"brightness":89,"light_switch":true}}' 

# 获取最新设备影子数据
./bin/tc_iot_shadow_cli.py GetIotShadow $cmd_cfg 

