#!/bin/bash

# 获取调用接口的密钥信息：
# https://console.qcloud.com/capi
SECRET_ID=${TENCENT_CLOUD_SECRET_ID}
SECRET_KEY=${TENCENT_CLOUD_SECRET_KEY}

# 获取产品ID和设备名称信息：
# https://console.qcloud.com/iotsuite/product
prodcut_id=iot-7hjcfc6k
device_name=light001


# light shadow 设备属性格式：
# {"name":"colorful light","color":16777215,"brightness":100,"light_switch":false}

cmd_cfg="--product_id=$prodcut_id --device_name=$device_name -u ${SECRET_ID} -p ${SECRET_KEY}"


# 通过控制台接口更新设备属性数据
# 开灯
echo control command '{"desired":{"device_switch":true}}'
./bin/tc_iot_shadow_cli.py UpdateIotShadow $cmd_cfg --shadow='{"desired":{"device_switch":true}}'  > /dev/null 
colors[0]=red
colors[1]=green
colors[2]=blue

while [ 1 ];
# 随机变化颜色
brightness=`expr $RANDOM % 100`
color=${colors[`expr $RANDOM % 3`]}
echo control command "{\"desired\":{\"color\":\"$color\",\"brightness\":$brightness}}"
do  ./bin/tc_iot_shadow_cli.py UpdateIotShadow $cmd_cfg --shadow="{\"desired\":{\"color\":\"$color\",\"brightness\":$brightness}}" > /dev/null;
test $? -gt 128 && break;
sleep 5
done

# 关灯
# ./bin/tc_iot_shadow_cli.py UpdateIotShadow $cmd_cfg --shadow='{"desired":{"light_switch":false}}'

# 获取最新设备数据
# ./bin/tc_iot_shadow_cli.py GetIotShadow $cmd_cfg 
