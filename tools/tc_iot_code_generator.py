#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import sys
import os
import six
import argparse

reload(sys)
sys.setdefaultencoding("utf-8")

try: import simplejson as json
except: import json
class TEMPLATE_CONSTANTS:
    TYPE = "type"
    AUTH_TYPE = "auth_type"
    NAME = "name"
    RANGE = "range"
    DATA_TEMPLATE = "data_template"

class AttributeDict(dict):
    def __getattr__(self, attr_name):
        result = self.__getitem__(attr_name)
        if result == None:
            print(attr_name + " test")
            pass
        return result

    #  __getattr__ = dict.__getitem__
    __setattr__ = dict.__setitem__

class code_template:
    def __init__(self, file_name, template):
        self.file_name = file_name
        self.template = template

class iot_enum:
    def __init__(self, parent, name, index):
        self.parent = parent
        self.name = name
        self.index = index
    def get_c_macro_name(self):
        return "TC_IOT_PROP_{}_{}".format(self.parent, self.name)

    def get_define_str(self):
        return "#define {} {}".format(self.get_c_macro_name(), self.index)

class iot_field:
    def __init__(self, name, index, field_obj):
        self.default_value = ""
        self.enums = []
        self.index = index
        self.name = name
        if TEMPLATE_CONSTANTS.TYPE not  in field_obj:
            raise ValueError("错误：{} 字段定义中未找到 Type 字段".format(name))

        self.type_name = field_obj[TEMPLATE_CONSTANTS.TYPE]

        if self.type_name == "bool":
            self.type_id = "TC_IOT_SHADOW_TYPE_BOOL"
            self.type_define = "tc_iot_shadow_bool"
            self.default_value = "false"
        elif self.type_name == "enum":
            self.type_id = "TC_IOT_SHADOW_TYPE_ENUM"
            self.type_define = "tc_iot_shadow_enum"
            if TEMPLATE_CONSTANTS.RANGE not in field_obj:
                raise ValueError("错误：{} 字段定义中未找到枚举定义 range 字段".format(name))

            enum_defs = field_obj[TEMPLATE_CONSTANTS.RANGE]
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
            if TEMPLATE_CONSTANTS.RANGE not in field_obj:
                raise ValueError("错误：{} 字段定义中未找到取值范围定义 Range 字段".format(name))
            if len(field_obj[TEMPLATE_CONSTANTS.RANGE]) != 2:
                raise ValueError("错误：{} 字段 Range 取值非法".format(name))

            self.min_value = field_obj[TEMPLATE_CONSTANTS.RANGE][0]
            self.max_value = field_obj[TEMPLATE_CONSTANTS.RANGE][1]
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
<indent>g_tc_iot_device_local_data.field_name = field_name;
<indent>if (field_name) {
<indent>    TC_IOT_LOG_TRACE("do something for field_name on");
<indent>} else {
<indent>    TC_IOT_LOG_TRACE("do something for field_name off");
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
<indent>g_tc_iot_device_local_data.field_name = field_name;
<indent>TC_IOT_LOG_TRACE("do something for field_name=%f", field_name);
""".replace("<indent>", indent).replace("field_name", self.name).replace("field_define", self.type_define)
        elif self.type_name == "int":
            sample_code = """
<indent>field_name = *(field_define *)data;
<indent>g_tc_iot_device_local_data.field_name = field_name;
<indent>TC_IOT_LOG_TRACE("do something for field_name=%d", field_name);
""".replace("<indent>", indent).replace("field_name", self.name).replace("field_define", self.type_define)
        else:
            raise Exception("invalid data type")

        return sample_code

    def get_data_change_sample_code_snippet(self, indent):
        sample_code = ""
        if self.type_name == "bool":
            sample_code = """
<indent>g_tc_iot_device_local_data.field_name = !g_tc_iot_device_local_data.field_name;
""".replace("<indent>", indent).replace("field_name", self.name)

        elif self.type_name == "enum":
            if (len(self.enums) > 1):
                sample_code += """
<indent>g_tc_iot_device_local_data.field_name += 1;
<indent>g_tc_iot_device_local_data.field_name %= <max>;
""".replace("<indent>", indent).replace("field_name", self.name).replace("<max>", str(len(self.enums)))

        elif self.type_name == "number":
            sample_code = """
<indent>g_tc_iot_device_local_data.field_name += 1;
<indent>g_tc_iot_device_local_data.field_name > <max>?<min>:g_tc_iot_device_local_data.field_name;
"""
            sample_code = sample_code.replace("<indent>", indent).replace("field_name", self.name)
            sample_code = sample_code.replace("field_define", self.type_define).replace("<min>",str(self.min_value)).replace("<max>",str(self.max_value))

        elif self.type_name == "int":
            sample_code = """
<indent>g_tc_iot_device_local_data.field_name += 1;
<indent>g_tc_iot_device_local_data.field_name > <max>?<min>:g_tc_iot_device_local_data.field_name;
"""
            sample_code = sample_code.replace("<indent>", indent).replace("field_name", self.name)
            sample_code = sample_code.replace("field_define", self.type_define).replace("<min>",str(self.min_value)).replace("<max>",str(self.max_value))
        else:
            raise Exception("invalid data type")

        return sample_code


class iot_struct:
    def __init__(self, obj):
        self.fields = []
        self.field_id = 0
        for field_define in obj:
            if TEMPLATE_CONSTANTS.NAME not in field_define:
                raise ValueError("错误：字段定义中未找到 Name 字段")
            self.fields.append(iot_field(field_define[TEMPLATE_CONSTANTS.NAME], self.field_id, field_define))
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

        sample_code += (indent * 1) + 'TC_IOT_LOG_TRACE("operating device");\n'
        sample_code += (indent * 1) + 'operate_device(&g_tc_iot_device_local_data);\n'
        sample_code += (indent * 1) + 'return TC_IOT_SUCCESS;\n'
        return declare_code + sample_code;

    def generate_sim_data_change(self):
        declare_code = ""
        sample_code = ""
        indent = "    "
        for field in self.fields:
            sample_code += field.get_data_change_sample_code_snippet(indent)
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
        header_str += "/* 数据模板本地存储结构定义 local data struct define */\n"
        header_str += "typedef struct _tc_iot_shadow_local_data {\n"
        for field in self.fields:
            header_str += "    {}\n".format(field.get_struct_field_declare())
        header_str += "}tc_iot_shadow_local_data;\n"

        header_str += "/* 数据模板字段 ID 宏定义*/\n"
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

def smart_parser(source_str, template_config, data_template, script_open_mark="/*${", script_close_mark="}*/"):
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
        code_snip = source_str[code_start+len(script_open_mark):code_end]
        temp = eval(code_snip)
        if (isinstance(temp, int)):
            result += str(temp)
        elif (isinstance(temp, six.string_types)):
            result += temp
        else:
            print("ERROR: unkonw data type return by:" + code_snip)

        start = code_end + len(script_close_mark)
        code_start = source_str.find(script_open_mark, start)
    result += source_str[start:]
    return result


def main():
    parser = argparse.ArgumentParser(description='Iotsuite device data code generator.')
    parser.add_argument('files', nargs='*')
    parser.add_argument('-c','--config', dest='config',metavar='iot-abcxyz.json', required=True,
                        help='配置文件本地路径，该文件可从控制台导出到本地： https://console.qcloud.com/iotsuite/product')

    args = parser.parse_args()

    config_path = args.config
    if not os.path.exists(config_path):
        print("错误：{} 文件不存在，请重新指定 device_config.json 文件路径".format(config_path))
        return 1

    config_dir = os.path.dirname(config_path)
    if config_dir:
        config_dir += "/"

    f = open(config_path, "r")
    try:
        device_config = json.load(f)
        device_config = AttributeDict(device_config)
        device_config.ProductId = device_config.product_id
        device_config.ProductKey = device_config.product_key
        device_config.Region = device_config.region
        device_config.AuthType = device_config.auth_type
        device_config.Domain = device_config.domain
        device_config.Username = device_config.username
        device_config.Password = device_config.password
        device_config.DataTemplate = device_config.data_template

        print("加载 {} 文件成功".format(config_path))
    except ValueError as e:
        print("错误：文件格式非法，请检查 {} 文件是否是 JSON 格式。".format(config_path))
        return 1

    if TEMPLATE_CONSTANTS.DATA_TEMPLATE not in device_config:
        print("错误：{} 文件中未发现 DataTemplate 属性字段，请检查文件格式是否合法。".format(config_path))
        return 1

    try:
        data_template = iot_struct(device_config.DataTemplate)
        for template_file in args.files:
            input_file_name = template_file
            input_file = open(input_file_name, "r")
            input_str = input_file.read()

            output_file_name = config_dir + os.path.basename(template_file)
            output_file = open(output_file_name, "w")
            output_file.write(smart_parser(input_str, device_config, data_template))
            print("文件 {} 生成成功".format(output_file_name))


        return 0
    except ValueError as e:
        print(e)
        return 1



if __name__ == '__main__':
    sys.exit(main())

