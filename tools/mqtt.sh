#!/bin/bash

DEVICE_SECRET=${TENCENT_CLOUD_DEMO_DEVICE_SECRET}
#./bin/demo_mqtt -s ${DEVICE_SECRET} --verbose

# test invalid host
#./bin/demo_mqtt -u test -P test -p 1883 --verbose -h invalid

# test invalid port
#./bin/demo_mqtt -u test -P test -p 1883 --verbose -h 127.0.0.1

# test invali username and password
#./bin/demo_mqtt -u test -P test


./bin/demo_mqtt -s ${DEVICE_SECRET} --verbose -p 1883

