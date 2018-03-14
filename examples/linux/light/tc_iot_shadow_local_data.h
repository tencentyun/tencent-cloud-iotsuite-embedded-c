#ifndef TC_IOT_SHADOW_LOCAL_DATA_H
#define TC_IOT_SHADOW_LOCAL_DATA_H

#include "tc_iot_inc.h"

#define TC_IOT_LIGHT_NAME_LEN  25

typedef struct _tc_iot_shadow_local_data {
    bool         light_switch; /* true means light on, false means light off. */
    char         name[TC_IOT_LIGHT_NAME_LEN];  /* name or id */
    unsigned int         color; /* ansi color index: 0~255, if greater use color=color%256 */
    unsigned int         brightness; /* light brightness*/
}tc_iot_shadow_local_data;


#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

typedef struct _tc_iot_shadow_control {
    tc_iot_shadow_local_data current;
    tc_iot_shadow_local_data reported;
} tc_iot_shadow_control;

#endif /* end of include guard */
