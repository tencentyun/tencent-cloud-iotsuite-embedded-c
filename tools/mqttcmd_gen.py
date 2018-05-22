#coding:utf-8
#import paho.mqtt.client as mqtt
'''
使用前先根据控制台产品信息设置环境变量
export TENCENT_CLOUD_DEMO_PRODUCT_ID="iot-i0ujhadi"
export TENCENT_CLOUD_DEMO_DEVICE_NAME="device_t1"
export TENCENT_CLOUD_DEMO_DEVICE_SECRET="xxxxxxxxxxxxxxxxx"
export TENCENT_CLOUD_DEMO_MQTT_HOST="mqtt-1ipy7vr68.ap-guangzhou.mqtt.tencentcloudmq.com"
export TENCENT_CLOUD_DEMO_MQTT_CID="mqtt-1ipy7vr68@device_t1"
'''
import time
import urllib
import urllib2
import contextlib
import hashlib
import hmac
import binascii
import json
from collections import OrderedDict
import os

'''
#这几行都没有用
product_id = 'iot-r3cyxxxx' 
product_key = 'mqtt-xxxxxxxx'
device_name = 'dev0001'
device_secret = 'xxxxxxxxxxxxxxxxxxxxxxx'
topic = '%s/%s/tp0001' % (product_id, device_name)
client_id = product_key + '@dev0001'
'''

product_id = os.getenv("TENCENT_CLOUD_DEMO_PRODUCT_ID")
mqtt_host =   os.getenv("TENCENT_CLOUD_DEMO_MQTT_HOST")
device_name = os.getenv("TENCENT_CLOUD_DEMO_DEVICE_NAME")
device_secret = os.getenv("TENCENT_CLOUD_DEMO_DEVICE_SECRET")
client_id =  os.getenv("TENCENT_CLOUD_DEMO_MQTT_CID")

product_key = mqtt_host.split(".")[0]

if not client_id or len(client_id) < 2 :
    client_id = product_key + '@' + device_name



params = OrderedDict([
    ['clientId', client_id],
    ['deviceName', device_name],
    ['expire', 72000],
    ['nonce', 123456],
    ['productId', product_id],
    ['timestamp', int(time.time())]])
params['signature'] = binascii.b2a_base64(hmac.new(device_secret, '&'.join(k + '=' + str(params[k]) for k in params), hashlib.sha256).digest())[:-1]




print "======================================================================="
print "POST BODY:" , urllib.urlencode(params)

request = urllib2.Request('http://gz.auth-device-iot.tencentcloudapi.com/device', urllib.urlencode(params))

with contextlib.closing(urllib2.urlopen(request, timeout=3)) as response:
    code = response.getcode()
    rsp = response.read()
    print "RESPONSE:" , code, rsp
    data = json.loads(rsp)['data']

print "-------------------------------------------------\n"

print 'examples/coap-client  -m post coap://122.152.224.121/auth -e "%s" \n' %  urllib.urlencode(params)
print 'mosquitto_sub  -h %s -p 1883 -u %s -P "%s" -i %s  -V mqttv311 -t "%s" -d\n'%( mqtt_host, data['id'], data['secret'] , client_id, "%s/%s/#" %(product_id, device_name) )
print 'mosquitto_pub  -h %s -p 1883 -u %s -P "%s" -i %s  -V mqttv311 -t "%s" -d -m "just4test" -q 1\n'%( mqtt_host, data['id'], data['secret'] , client_id, "%s/%s/XXXXX_PLS_MODIFY" %(product_id, device_name) )
