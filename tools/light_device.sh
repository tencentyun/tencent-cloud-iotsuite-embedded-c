#!/bin/bash

# 控制台获取设备 Device Secret:
# https://console.qcloud.com/iotsuite/product
DEVICE_SECRET=${TENCENT_CLOUD_LIGHT_DEVICE_SECRET}

./bin/scn_light -d light001 -s ${DEVICE_SECRET} -p 1883 --trace

