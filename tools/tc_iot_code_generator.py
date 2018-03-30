#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import sys
import os

reload(sys)
sys.setdefaultencoding("utf-8")

try: import simplejson as json
except: import json

class code_template:
    file_name = ""
    template = ""
    def __init__(self, file_name, template):
        self.file_name = file_name
        self.template = template

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
        if "Type" not  in field_obj:
            raise ValueError("错误：{} 字段定义中未找到 Type 字段".format(name))

        self.type_name = field_obj["Type"]

        if self.type_name == "bool":
            self.type_id = "TC_IOT_SHADOW_TYPE_BOOL"
            self.type_define = "tc_iot_shadow_bool"
            self.default_value = "false"
        elif self.type_name == "enum":
            self.type_id = "TC_IOT_SHADOW_TYPE_ENUM"
            self.type_define = "tc_iot_shadow_enum"
            if "Range" not in field_obj:
                raise ValueError("错误：{} 字段定义中未找到枚举定义 range 字段".format(name))

            enum_defs = field_obj['Range']
            enum_id = 0
            for enum_name in enum_defs:
                current_enum = iot_enum(self.name, enum_name, enum_id)
                self.enums.append(current_enum)
                if self.default_value == "":
                    self.default_value = current_enum.get_c_macro_name()
                enum_id += 1
            if self.default_value == "":
                raise ValueError("错误：{} 字段默认值 {} 非法".format(name, field_obj["default"]))

        elif self.type_name == "number":
            self.type_id = "TC_IOT_SHADOW_TYPE_NUMBER"
            self.type_define = "tc_iot_shadow_number"
            if "Range" not in field_obj:
                raise ValueError("错误：{} 字段定义中未找到取值范围定义 Range 字段".format(name))
            if len(field_obj["Range"]) != 2:
                raise ValueError("错误：{} 字段 Range 取值非法".format(name))

            self.min_value = field_obj['Range'][0]
            self.max_value = field_obj['Range'][1]
            self.default_value = self.min_value
            if self.default_value < self.min_value or self.default_value > self.max_value:
                raise ValueError("错误：{} 字段 default 指定的默认值超出 min~max 取值范围".format(name))
        else:
            raise ValueError('{} 字段 数据类型 type={} 取值非法，有效值应为：bool,enum,number'.format(name, field_obj["type"]))

    def get_id_c_macro_name(self):
        return "TC_IOT_PROP_{}".format(self.name)

    def get_id_define_str(self):
        return "#define {} {}".format(self.get_id_c_macro_name(), self.index)

    def get_struct_field_declare(self):
        return "{} {};".format(self.type_define, self.name)

    def get_meta_define_str(self):
        return '{{ "{}", {}, {}, offsetof(tc_iot_shadow_local_data, {}) }},'.format(self.name, self.get_id_c_macro_name(), self.type_id, self.name)

    def get_sample_code_snippet(self, indent, data_pointer):
        sample_code = ""
        if self.type_name == "bool":
            sample_code = """
<indent>field_name = *(field_define *)data;
<indent>if (field_name) {
<indent>    TC_IOT_LOG_TRACE("do something for field_name on");
<indent>} else {
<indent>    TC_IOT_LOG_TRACE("do something for field_name off");
<indent>}
""".replace("<indent>", indent).replace("field_name", self.name).replace("field_define", self.type_define)

        elif self.type_name == "enum":
            sample_code = """
<indent>field_name = *(field_define *)data;
""".replace("<indent>", indent).replace("field_name", self.name).replace("field_define", self.type_define)
            sample_code += indent + "switch({})".format(self.name) + "{\n"
            for enum in self.enums:
                sample_code += indent + "    case {}:\n".format(enum.get_c_macro_name())
                sample_code += indent + '        TC_IOT_LOG_TRACE("do something for {} = {}");\n'.format(self.name, enum.name)
                sample_code += indent + "        break;\n"

            sample_code += indent + "    default:\n"
            sample_code += indent + '        TC_IOT_LOG_WARN("do something for {} = unknown");\n'.format(self.name)
            sample_code += indent + "        /* 如果能正常处理未知状态，则返回 TC_IOT_SUCCESS */\n"
            sample_code += indent + "        /* 如果不能正常处理未知状态，则返回 TC_IOT_FAILURE */\n"
            sample_code += indent + "        return TC_IOT_FAILURE;\n"
            sample_code += indent + "}\n"

        elif self.type_name == "number":
            sample_code = """
<indent>field_name = *(field_define *)data;
<indent>TC_IOT_LOG_TRACE("do something for field_name=%d", field_name);
""".replace("<indent>", indent).replace("field_name", self.name).replace("field_define", self.type_define)
        else:
            raise Exception("invalid data type")

        return sample_code


class iot_struct:
    fields = []
    field_id = 0
    def __init__(self, obj):
        for field_define in obj:
            if "Name" not in field_define:
                raise ValueError("错误：字段定义中未找到 Name 字段")
            self.fields.append(iot_field(field_define['Name'], self.field_id, field_define))
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
        sample_code += (indent * 3) + 'TC_IOT_LOG_WARN("unkown property id = %d", property_id);\n'
        sample_code += (indent * 3) + 'return TC_IOT_FAILURE;\n'
        sample_code += (indent * 1) + '}\n\n'

        # /* 上报所有状态 */
        # /* tc_iot_report_DataTemplate(  */
        # /* 3 */
        # /* ,TC_IOT_PROP_device_switch , &g_tc_iot_device_local_data.device_switch */
        # /* ,TC_IOT_PROP_color , &g_tc_iot_device_local_data.color */
        # /* ,TC_IOT_PROP_brightness , &g_tc_iot_device_local_data.brightness */
        # /* ); */

        sample_code += (indent * 1) + 'TC_IOT_LOG_TRACE("operating device");\n'
        sample_code += (indent * 1) + 'operate_device(&g_tc_iot_device_local_data);\n'
        sample_code += (indent * 1) + 'return TC_IOT_SUCCESS;\n'
        return declare_code + sample_code;

    def declare_local_data_struct(self, struct_name="tc_iot_shadow_local_data"):
        result = ""
        result += "typedef struct _" + struct_name + " {\n"
        for field in self.fields:
            result += "    {}\n".format(field.get_struct_field_declare())
        result += "}" + struct_name + ";\n"
        return result

    def declare_local_data_field_id(self):
        result = ""
        for field in self.fields:
            result += "{}\n".format(field.get_id_define_str())
	result += "\n#define TC_IOT_PROPTOTAL {}\n".format(self.field_id)
        return result

    def declare_local_data_enum(self):
        result = ""
        for field in self.fields:
            if field.type_name == 'enum':
                result += "\n/* enum macro definition for {} */\n".format(field.name)
                for enum in field.enums:
                    result += "{}\n".format(enum.get_define_str())
        return result

    def generate_header(self):
        header_template = """#ifndef TC_IOT_DEVICE_LOGIC_H
#define TC_IOT_DEVICE_LOGIC_H

#include "tc_iot_inc.h"

/*<header_str>*/

tc_iot_shadow_client * tc_iot_get_shadow_client(void);
#endif /* end of include guard */
"""
        header_str = ""
        header_str += "/* 数据点本地存储结构定义 local data struct define */\n"
        header_str += "typedef struct _tc_iot_shadow_local_data {\n"
        for field in self.fields:
            header_str += "    {}\n".format(field.get_struct_field_declare())
        header_str += "}tc_iot_shadow_local_data;\n"

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

    def property_def_initializer(self):
        meta_define_str = ""
        for field in self.fields:
            meta_define_str += "    {}\n".format(field.get_meta_define_str())
        return meta_define_str;

    def local_data_initializer(self):
        default_init_str = ""
        for field in self.fields:
            default_init_str += "    {},\n".format(field.default_value)
        return default_init_str


    def generate_source(self):

        sample_code = self.generate_sample_code()

        return source_template_str \
            .replace("/*<meta_define_str>*/", meta_define_str) \
            .replace("/*<default_init_str>*/", default_init_str) \
            .replace("/*<sample_code>*/", sample_code) \

def smart_parser(source_str, data_template, script_open_mark="/*${", script_close_mark="}*/"):
    test = "1"
    str = "hello"
    start = 0
    code_start = 0
    code_end = 0
    result = ""

    code_start = source_str.find(script_open_mark, start)
    while(code_start >= 0):
        code_end = source_str.find(script_close_mark, code_start)
        if (code_end <= code_start):
            print("ERROR: " + script_open_mark + " has no match " + script_close_mark + "\n")
            break
        result += source_str[start:code_start]
        temp = eval(source_str[code_start+len(script_open_mark):code_end])
        result += temp
        start = code_end + len(script_close_mark)
        code_start = source_str.find(script_open_mark, start)
    result += source_str[start:]
    return result


def main():
    if len(sys.argv) <= 1:
        print sys.argv[0] + ''' 是数据点定义及代码框架生成脚本，基于 Python 开发，需要在 Python 环境下运行。
本工具以数据点定义文件 device_config.json (https://console.qcloud.com/iotsuite/product) 为输入，根据该文件的定义，自动生成相关 c 代码。'''
        print "执行命令：\n" + sys.argv[0] + " device_config.json"
        return 0
    else:
        #  try:
            template_path = sys.argv[1]
            if not os.path.exists(template_path):
                print("错误：{} 文件不存在，请重新指定 device_config.json 文件路径".format(template_path))
                return 1

            template_dir = os.path.dirname(template_path)
            if template_dir:
                template_dir += "/"

            f = open(template_path, "r")
            try:
                device_config = json.load(f)
                print("加载 {} 文件成功".format(template_path))
            except ValueError as e:
                print("错误：文件格式非法，请检查 {} 文件是否是 JSON 格式。".format(template_path))
                return 1

            if "DataTemplate" not in device_config:
                print("错误：{} 文件中未发现 DataTemplate 属性字段，请检查文件格式是否合法。".format(template_path))
                return 1

            try:
                data_template = iot_struct(device_config["DataTemplate"])

                templates = [
code_template("tc_iot_device_logic.h",
"""#ifndef TC_IOT_DEVICE_LOGIC_H
#define TC_IOT_DEVICE_LOGIC_H

#include "tc_iot_inc.h"

/* 数据点本地存储结构定义 local data struct define */
/*${data_template.declare_local_data_struct()}*/

/* 数据点字段 ID 宏定义*/
/*${data_template.declare_local_data_field_id()}*/

/* enum macro definitions */
/*${data_template.declare_local_data_enum()}*/

tc_iot_shadow_client * tc_iot_get_shadow_client(void);
#endif /* end of include guard */
"""),

code_template("tc_iot_device_logic.c",
"""#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client,  void * context);
void operate_device(tc_iot_shadow_local_data * device);


/* 影子数据 Client  */
tc_iot_shadow_client g_tc_iot_shadow_client;

tc_iot_shadow_client * tc_iot_get_shadow_client(void) {
    return &g_tc_iot_shadow_client;
}


/* 设备本地数据类型及地址、回调函数等相关定义 */
tc_iot_shadow_property_def g_tc_iot_shadow_property_defs[] = {
/*${ data_template.property_def_initializer() }*/};


/* 设备当前状态数据 */
tc_iot_shadow_local_data g_tc_iot_device_local_data = {
/*${ data_template.local_data_initializer() }*/};

/* 设备状态控制数据 */
static tc_iot_shadow_local_data g_tc_iot_device_desired_data = {
/*${ data_template.local_data_initializer() }*/};

/* 设备已上报状态数据 */
tc_iot_shadow_local_data g_tc_iot_device_reported_data = {
/*${ data_template.local_data_initializer() }*/};

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
    TC_IOT_PROPTOTAL,
    &g_tc_iot_shadow_property_defs[0],
    _tc_iot_shadow_property_control_callback,
    &g_tc_iot_device_local_data,
    &g_tc_iot_device_reported_data,
    &g_tc_iot_device_desired_data,
};


static int _tc_iot_property_change( int property_id, void * data) {
/*${data_template.generate_sample_code()}*/
}

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client,  void * context) {
    tc_iot_shadow_property_def * p_property = NULL;

    if (!msg) {
        TC_IOT_LOG_ERROR("msg is null.");
        return TC_IOT_FAILURE;
    }

    if (msg->event == TC_IOT_SHADOW_EVENT_SERVER_CONTROL) {
        p_property = (tc_iot_shadow_property_def *)context;
        if (!p_property) {
            TC_IOT_LOG_ERROR("p_property is null.");
            return TC_IOT_FAILURE;
        }

        return _tc_iot_property_change(p_property->id, msg->data);
    } else if (msg->event == TC_IOT_SHADOW_EVENT_REQUEST_REPORT_FIRM) {
        tc_iot_report_firm(tc_iot_get_shadow_client(),
                "product", TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
                "device", TC_IOT_CONFIG_DEVICE_NAME,
                "sdk-ver", "1.0",
                "firm-ver","1.0", NULL);
    } else {
        TC_IOT_LOG_TRACE("unkown event received, event=%d", msg->event);
    }
    return TC_IOT_SUCCESS;
}

""")]

                for template in templates:
                    output_file_name = template_dir + template.file_name
                    output_file = open(output_file_name, "w")
                    output_file.write(smart_parser(template.template, data_template))
                    print "文件 {} 生成成功".format(output_file_name)


                return 0
            except ValueError as e:
                print(e)
                return 1

        #  except Exception as e:
            #  #  print e
            #  raise e
            #  return 1


if __name__ == '__main__':
    sys.exit(main())

