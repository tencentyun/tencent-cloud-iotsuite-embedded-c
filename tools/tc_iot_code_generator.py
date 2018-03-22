#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import sys

reload(sys)
sys.setdefaultencoding("utf-8")

try: import simplejson as json
except: import json

class iot_enum:
    parent = ""
    name = ""
    index = 0

    def __init__(self, parent, name, index):
        self.parent = parent
        self.name = name
        self.index = index
    def get_c_macro_name(self):
        return "TC_IOT_PROP_{}_{}".format(self.parent, self.name)

    def get_define_str(self):
        return "#define {} {}".format(self.get_c_macro_name(), self.index)

class iot_field:
    name = ""
    index = 0
    type_name = ""
    type_id = ""
    type_define = ""
    default_value = ""
    min_value = ""
    max_value = ""
    enums = []

    def __init__(self, name, index, field_obj):
        self.index = index
        self.name = name
        self.type_name = field_obj["type"]

        if self.type_name == "bool":
            self.type_id = "TC_IOT_SHADOW_TYPE_BOOL"
            self.type_define = "tc_iot_shadow_bool"
            if field_obj["default"]:
                self.default_value = "true" 
            else:
                self.default_value = "false"
        elif self.type_name == "enum":
            self.type_id = "TC_IOT_SHADOW_TYPE_ENUM"
            self.type_define = "tc_iot_shadow_enum"
            enum_defs = field_obj['range'].split(',')
            enum_id = 0
            for enum_name in enum_defs:
                current_enum = iot_enum(self.name, enum_name, enum_id)
                self.enums.append(current_enum)
                if (enum_name == field_obj["default"]):
                    self.default_value = current_enum.get_c_macro_name()
                enum_id += 1
        elif self.type_name == "number":
            self.type_id = "TC_IOT_SHADOW_TYPE_NUMBER"
            self.type_define = "tc_iot_shadow_number"
            self.min_value = field_obj['min']
            self.max_value = field_obj['max']
            self.default_value = field_obj["default"]
        else:
            raise Exception(k, '"{}" 字段 数据类型 type={} 取值非法'.format(k, v["type"]))

    def get_id_c_macro_name(self):
        return "TC_IOT_PROP_{}".format(self.name)

    def get_id_define_str(self):
        return "#define {} {}".format(self.get_id_c_macro_name(), self.index)

    def get_struct_define_str(self):
        return "{} {};".format(self.type_define, self.name)

    def get_meta_define_str(self):
        return 'DECLARE_PROPERTY_DEF({}, {}),'.format(self.name, self.type_id)
    def get_sample_code_snippet(self, indent, data_pointer):
        sample_code = ""
        if self.type_name == "bool":
            sample_code = """
<indent>field_name = *(field_define *)data;
<indent>g_tc_iot_device_local_data.field_name = field_name;
<indent>if (field_name) {
<indent>    LOG_TRACE("do something for field_name on");
<indent>} else {
<indent>    LOG_TRACE("do something for field_name off");
<indent>}
""".replace("<indent>", indent).replace("field_name", self.name).replace("field_define", self.type_define)

        elif self.type_name == "enum":
            sample_code = """
<indent>field_name = *(field_define *)data;
<indent>g_tc_iot_device_local_data.field_name = field_name;
""".replace("<indent>", indent).replace("field_name", self.name).replace("field_define", self.type_define)
            sample_code += indent + "switch({})".format(self.name) + "{\n"
            for enum in self.enums:
                sample_code += indent + "    case {}:\n".format(enum.get_c_macro_name())
                sample_code += indent + '        LOG_TRACE("do something for {} = {}");\n'.format(self.name, enum.name)
                sample_code += indent + "        break;\n"

            sample_code += indent + "    default:\n"
            sample_code += indent + "        break;\n"
            sample_code += indent + "}\n"

        elif self.type_name == "number":
            sample_code = """
<indent>field_name = *(field_define *)data;
<indent>g_tc_iot_device_local_data.field_name = field_name;
<indent>LOG_TRACE("do something for field_name=%d", field_name);
""".replace("<indent>", indent).replace("field_name", self.name).replace("field_define", self.type_define)
        else:
            raise Exception("invalid data type")

        return sample_code


class iot_struct:
    fields = []
    field_id = 0
    def __init__(self, obj):
        for field_name,field_define in obj.items():
            self.fields.append(iot_field(field_name, self.field_id, field_define))
            self.field_id += 1

    def generate_sample_code(self):
        declare_code = ""
        indent = "    "
        sample_code = (indent * 1) + "switch (property_id) {\n"
        for field in self.fields:
            declare_code += ((indent * 1) + "{} {};\n").format(field.type_define, field.name)
            sample_code += (indent * 2) + "case {}:".format(field.get_id_c_macro_name())
            sample_code += field.get_sample_code_snippet(indent*3, "data")
            sample_code +=  (indent * 3) + "break;\n"
        sample_code += (indent * 2) + "default:\n"
        sample_code += (indent * 3) + 'LOG_WARN("unkown property id = %d", property_id);\n'
        sample_code += (indent * 3) + 'return TC_IOT_FAILURE;\n'
        sample_code += (indent * 1) + '}\n\n'

        # /* 上报所有状态 */
        # /* tc_iot_shadow_update_reported_propeties(  */
        # /* 3 */
        # /* ,TC_IOT_PROP_device_switch , &g_tc_iot_device_local_data.device_switch */
        # /* ,TC_IOT_PROP_color , &g_tc_iot_device_local_data.color */
        # /* ,TC_IOT_PROP_brightness , &g_tc_iot_device_local_data.brightness */
        # /* ); */

        sample_code += (indent * 1) + 'tc_iot_shadow_update_reported_propeties( 1, property_id, data);\n'
        sample_code += (indent * 1) + 'LOG_TRACE("operating device");\n'
        sample_code += (indent * 1) + 'operate_device(&g_tc_iot_device_local_data);\n'
        sample_code += (indent * 1) + 'return TC_IOT_SUCCESS;\n'
        return declare_code + sample_code;

    def generate_header(self):
        header_template = """#ifndef TC_IOT_DEVICE_LOGIC_H
#define TC_IOT_DEVICE_LOGIC_H

#include "tc_iot_inc.h"

/*<header_str>*/

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client,  void * context);

extern tc_iot_shadow_client g_tc_iot_shadow_client;
extern tc_iot_shadow_property_def g_device_property_defs[];
#define DECLARE_PROPERTY_DEF(name, type) {#name, TC_IOT_PROP_ ## name, type}


#endif /* end of include guard */
"""
        header_str = ""
        header_str += "/* 数据点本地存储结构定义 local data struct define */\n"
        header_str += "typedef struct _tc_iot_shadow_local_data {\n"
        for field in self.fields:
            header_str += "    {}\n".format(field.get_struct_define_str())
        header_str += "};\n"

	header_str += "/* 数据点字段 ID 宏定义*/\n"
        for field in self.fields:
            header_str += "{}\n".format(field.get_id_define_str())
	header_str += "\n#define TC_IOT_PROPTOTAL {}\n".format(self.field_id)

        for field in self.fields:
            if field.type_name == 'enum':
                header_str += "\n/* enum macro definition for {} */\n".format(field.name)
                for enum in field.enums:
                    header_str += "{}\n".format(enum.get_define_str())
        return header_template.replace("/*<header_str>*/",header_str);

    def generate_source(self):
        source_template_str = """#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client,  void * context);
void operate_device(tc_iot_shadow_local_data * device);

#define DECLARE_PROPERTY_DEF(name, type) {#name, TC_IOT_PROP_ ## name, type}

/* 设备本地数据类型及地址、回调函数等相关定义 */
tc_iot_shadow_property_def g_tc_iot_shadow_property_defs[] = {
/*<meta_define_str>*/
};

/* 设备初始配置 */
tc_iot_shadow_config g_tc_iot_shadow_config = {
    {
        {
            /* device info*/
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
            TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
        },
        TC_IOT_CONFIG_SERVER_HOST,
        TC_IOT_CONFIG_SERVER_PORT,
        TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
        TC_IOT_CONFIG_TLS_HANDSHAKE_TIMEOUT_MS,
        TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
        TC_IOT_CONFIG_CLEAN_SESSION,
        TC_IOT_CONFIG_USE_TLS,
        TC_IOT_CONFIG_AUTO_RECONNECT,
        TC_IOT_CONFIG_ROOT_CA,
        TC_IOT_CONFIG_CLIENT_CRT,
        TC_IOT_CONFIG_CLIENT_KEY,
        NULL,
        NULL,
        0,  /* send will */
        {
            {'M', 'Q', 'T', 'W'}, 0, {NULL, {0, NULL}}, {NULL, {0, NULL}}, 0, 0,
        }
    },
    TC_IOT_SUB_TOPIC_DEF,
    TC_IOT_PUB_TOPIC_DEF,
    tc_iot_device_on_message_received,
    TC_IOT_PROP_TOTAL,
    &g_tc_iot_shadow_property_defs[0],
    _tc_iot_shadow_property_control_callback,
};


/* 设备状态数据 */
static tc_iot_shadow_local_data g_tc_iot_device_local_data = {
/*<default_init_str>*/};


static int _tc_iot_property_change( int property_id, void * data) {
/*<sample_code>*/
}

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client,  void * context) {
    tc_iot_shadow_property_def * p_property = NULL;
    tc_iot_shadow_bool device_switch = false;
    tc_iot_shadow_enum color  = TC_IOT_PROP_color_red;
    tc_iot_shadow_number brightness  = 0;

    if (!msg) {
        LOG_ERROR("msg is null.");
        return TC_IOT_FAILURE;
    }

    if (msg->event == TC_IOT_SHADOW_EVENT_SERVER_CONTROL) {
        p_property = (tc_iot_shadow_property_def *)context;
        if (!p_property) {
            LOG_ERROR("p_property is null.");
            return TC_IOT_FAILURE;
        }

        return _tc_iot_property_change(p_property->id, msg->data);
    } else if (msg->event == TC_IOT_SHADOW_EVENT_REQUEST_REPORT_FIRM) {
        /* tc_iot_report_firm(3, "mac","00-00-00-00-00", "sdk-ver", "1.0", "firm-ver","2.0.20180123.pre"); */
    } else {
        LOG_TRACE("unkown event received, event=%ds", msg->event);
    }
    return TC_IOT_SUCCESS;
}

"""
        meta_define_str = ""
        for field in self.fields:
            meta_define_str += "    {}\n".format(field.get_meta_define_str())

        default_init_str = ""
        for field in self.fields:
            default_init_str += "    {},\n".format(field.default_value)

        sample_code = self.generate_sample_code()

        return source_template_str \
            .replace("/*<meta_define_str>*/", meta_define_str) \
            .replace("/*<default_init_str>*/", default_init_str) \
            .replace("/*<sample_code>*/", sample_code) \



def main():
    if len(sys.argv) <= 1:
        print "ERROR: 处理失败，请指定文件路径，说明："
        print sys.argv[0] + " template.json"
        print ''' template.json 格式大致如下：
{
    "数据点1":{"type":"bool","default":false}, // 布尔类型，false 和 true 两个取值，用来定义开关状态。
    "数据点2":{"type":"enum", "range":"enum1,enum2,enumN","default":"enum1"}, // 枚举类型，range 定义枚举，多个枚举之间用","分隔。
    "数据点3":{"type":"number","min":0,"max":4096,"default":0} // 数值类型，min、max 用来定义数值取值范围。
}
例如：
{
    "device_switch":{"type":"bool","default":true},
    "color":{"type":"enum", "range":"red,green,blue","default":"green"},
    "brightness":{"type":"number","min":0,"max":100, "default":1}
}
'''
        return 0
    else:
        # try:
            f = open(sys.argv[1], "r")
            defs = json.load(f)

            st = iot_struct(defs)
            # print st.generate_header()
            # print logic_header
            header_file_name = "tc_iot_device_logic.h"
            header_file = open(header_file_name, "w")
            header_file.write(st.generate_header())
            print "generate {} success.".format(header_file_name)

            source_file_name = "tc_iot_device_logic.c"
            source_file = open(source_file_name, "w")
            source_file.write(st.generate_source())
            print "generate {} success.".format(source_file_name)

            return 0
        # except Exception as e:
            # print e
            # return 1


if __name__ == '__main__':
    sys.exit(main())


