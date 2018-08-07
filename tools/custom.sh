#!/bin/bash

DEVICE_SECRET=${TENCENT_CLOUD_DEMO_DEVICE_SECRET}

./bin/basic_mqtt_binary -s ${DEVICE_SECRET} --trace --port=1883

