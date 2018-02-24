#!/bin/bash

DEVICE_SECRET=${TENCENT_CLOUD_DEMO_DEVICE_SECRET}

./bin/demo_shadow -s ${DEVICE_SECRET} --trace -p 1883

# gdb --args ./bin/demo_shadow -s ${DEVICE_SECRET} --trace -p 1883

# test invalid host
# ./bin/demo_shadow -u test -P test -p 1883 --verbose -h invalid

# test invalid port
# ./bin/demo_shadow -u test -P test -p 1883 --verbose -h 127.0.0.1

# test invali username and password
# ./bin/demo_shadow -u test -P test
