#ifndef TC_IOT_DEVICE_LOGIC_H
#define TC_IOT_DEVICE_LOGIC_H

#include "tc_iot_inc.h"

/* 数据模板本地存储结构定义 local data struct define */
/*${data_template.declare_local_data_struct()}*/

/* 数据模板字段 ID 宏定义*/
/*${data_template.declare_local_data_field_id()}*/

/* enum macro definitions */
/*${data_template.declare_local_data_enum()}*/

tc_iot_shadow_client * tc_iot_get_shadow_client(void);
#endif /* end of include guard */
