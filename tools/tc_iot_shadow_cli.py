#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import hashlib
import urllib
import requests
import binascii
import hmac
import copy
import random
import sys
import time
from pprint import pprint
from optparse import OptionParser
from requests.packages.urllib3.exceptions import InsecurePlatformWarning
requests.packages.urllib3.disable_warnings(InsecurePlatformWarning)
from requests.packages.urllib3.exceptions import InsecureRequestWarning
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

reload(sys)
sys.setdefaultencoding("utf-8")

try: import simplejson as json
except: import json


class Sign:
    def __init__(self, secretId, secretKey):
        self.secretId = secretId
        self.secretKey = secretKey

    def make(self, requestHost, requestUri, params, method = 'GET'):
        srcStr = method.upper() + requestHost + requestUri + '?' + "&".join(k.replace("_",".") + "=" + str(params[k]) for k in sorted(params.keys()))
        hashed = hmac.new(self.secretKey, srcStr, hashlib.sha1)
        return binascii.b2a_base64(hashed.digest())[:-1]

class Request:
    timeout = 10
    version = 'Python_Tools'
    def __init__(self, secretId, secretKey):
        self.secretId = secretId
        self.secretKey = secretKey

    def send(self, requestHost, requestUri, params, files = {}, method = 'GET', debug = 0):
        params['RequestClient'] = Request.version
        params['SecretId'] = self.secretId
        sign = Sign(self.secretId, self.secretKey)
        params['Signature'] = sign.make(requestHost, requestUri, params, method)

        url = 'https://%s%s' % (requestHost, requestUri)

        if debug:
            print method.upper(), url
            print 'Request Args:'
            pprint(params)
        start =time.time()
        if method.upper() == 'GET':
            req = requests.get(url, params=params, timeout=Request.timeout,verify=False)
        else:
            req = requests.post(url, data=params, files=files, timeout=Request.timeout,verify=False)
        print '{'
        print '"elapsed_time":' ,str((time.time() - start) * 1000) , ","
        print '"http_status_code":' , req.status_code , ","
        print '"response":'
        print req.text
        print "}"
        if req.status_code != requests.codes.ok:
            req.raise_for_status()


def Name(name):
    up = False
    new_name = ""
    for i in name:
        if i == '_':
            up = True
            continue
        if up:
            new_name += i.upper()
        else:
            new_name += i
        up = False
    return new_name


class Iot:
    def __init__(self):
        self.params = {
                'Region': 'gz',
                'Nonce': random.randint(1, sys.maxint),
                'Timestamp': int(time.time()),
                }
        self.files = {}
        self.host = 'iot.api.qcloud.com'
        self.uri = '/v2/index.php'
        self.method = "POST"
        self.debug = 1

    def parse_args(self):
        actions = []
        for method in dir(self):
            if method[0].isupper():
                actions.append( method )

        usage='usage: %prog Action [options]\nThis is a command line tools to access Qcloud API.\n\nSupport Actions:\n    '+"\n    ".join(actions)
        self.parser = OptionParser(usage=usage)
        from sys import argv
        if len(argv) < 2 or argv[1] not in actions:
            self.parser.print_help()
            return 0

        action = argv[1]
        self.params['Action'] = action
        usage='usage: %%prog Action [options]\n\nThis is help message for action "%s"\nMore Usage: http://www.qcloud.com/wiki/v2/%s' % (action, action)
        self.parser = OptionParser(usage=usage)
        self.parser.add_option('--debug', dest='debug', action="store_true", default=False, help='Print debug message')
        self.parser.add_option('-u', '--secret_id', dest='secret_id', help='Secret ID from <https://console.qcloud.com/capi>')
        self.parser.add_option('-p', '--secret_key', dest='secret_key', help='Secret Key from <https://console.qcloud.com/capi>')
        getattr(self, action)()
        if len(argv) == 2:
            self.parser.print_help()
            return 0

        (options, args) = self.parser.parse_args() # parse again
        self.debug = options.debug
        for key in dir(options):
            if not key.startswith("__") and getattr(options, key) is None:
                raise KeyError, ('Error: Please provide options --%s' % key)


        for option in self.parser.option_list:
            opt = option.dest
            if opt not in [None, 'secret_id', 'secret_key', 'debug']:
                self.params[ Name(opt) ] = getattr(options, opt)

        self.options = options
        method = 'get_params_' + action
        if hasattr(self, method): getattr(self, method)()

        # format params
        for key, value in self.params.items():
            if value == '':
                del self.params[key]
            if isinstance(value, list):
                del self.params[key]
                for idx, val in enumerate(value):
                    self.params["%s.%s"%(key, idx)] = val

        request = Request(options.secret_id, options.secret_key)
        return request.send(self.host, self.uri, self.params, self.files, self.method, self.debug)


    def GetIotShadow(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--device_name', dest='deviceName', default='',help="device name, from <https://console.qcloud.com/iotsuite/product>")

    def UpdateIotShadow(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--device_name', dest='deviceName', default='',help="device name, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--shadow', dest='shadow', default='{"reported":{},"desired":{}}',help='Shadow document json format, like this:\n{"reported":{"device":"on"},"desired":{"device":"off"}}')

    def DeleteIotDevice(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--device_name', dest='deviceName', default='',help="device name, from <https://console.qcloud.com/iotsuite/product>")

    def GetIotDeviceList(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--offset', dest='offset', default='0')
        self.parser.add_option('--length', dest='length', default='10')

    def GetIotDevice(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--device_name', dest='deviceName', default='',help="device name, from <https://console.qcloud.com/iotsuite/product>")

    def GetIotDeviceData(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--device_name', dest='deviceName', default='',help="device name, from <https://console.qcloud.com/iotsuite/product>")

    def IssueIotDeviceControl(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--device_name', dest='deviceName', default='',help="device name, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--control_data', dest='controlData', default='{"device_switch":true}', \
                               help='Shadow document json format, like this:\n{"device_switch":true,"color":"green","brightness":80}')

    def CreateIotDevice(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")
        self.parser.add_option('--device_name', dest='deviceName', default='',help="device name, from <https://console.qcloud.com/iotsuite/product>")

    def GetIotProductList(self):
        self.parser.add_option('--offset', dest='offset', default='0')
        self.parser.add_option('--length', dest='length', default='10')
        self.parser.add_option('--include_deleted', dest='includeDeleted', default='0')

    def GetIotProduct(self):
        self.parser.add_option('--product_id', dest='productId', default='', help="product id, from <https://console.qcloud.com/iotsuite/product>")

    def CreateIotProduct(self):
        self.parser.add_option('--name', dest='name', default='', help="product name")
        self.parser.add_option('--description', dest='description', default='', help="product description")
        self.parser.add_option('--auth_type', dest='authType', default='', help="连接模式（0：直连，1：token）")


def main():
    iot = Iot()
    try:
        iot.parse_args()
    except Exception as e:
        print e
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())

