#ifndef TC_IOT_DEVICE_LOGIC_H
#define TC_IOT_DEVICE_LOGIC_H

#include "tc_iot_inc.h"

/* 数据模板本地存储结构定义 local data struct define */
typedef struct _tc_iot_shadow_local_data {
    tc_iot_shadow_bool device_switch;
    tc_iot_shadow_enum color;
    tc_iot_shadow_number brightness;
    tc_iot_shadow_number power;
    char name[16+1];
}tc_iot_shadow_local_data;


/* 数据模板字段 ID 宏定义*/
#define TC_IOT_PROP_device_switch 0
#define TC_IOT_PROP_color 1
#define TC_IOT_PROP_brightness 2
#define TC_IOT_PROP_power 3
#define TC_IOT_PROP_name 4

#define TC_IOT_PROPTOTAL 5


/* enum macro definitions */

/* enum macro definition for color */
#define TC_IOT_PROP_color_red 0
#define TC_IOT_PROP_color_green 1
#define TC_IOT_PROP_color_blue 2


tc_iot_shadow_client * tc_iot_get_shadow_client(void);
#endif /* end of include guard */
