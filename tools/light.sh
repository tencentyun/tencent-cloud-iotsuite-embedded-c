#!/bin/bash

DEVICE_SECRET=${TENCENT_CLOUD_LIGHT_DEVICE_SECRET}
# gdb --args ./bin/light_device -s ${DEVICE_SECRET} -p 1883 --trace
./bin/light_device -s ${DEVICE_SECRET} -p 1883 --trace

