#ifndef TC_IOT_DEMO_LIGHT_H
#define TC_IOT_DEMO_LIGHT_H

#include "tc_iot_inc.h"

#define TC_IOT_LIGHT_NAME_LEN  25

typedef struct _tc_iot_demo_light {
    bool          light_switch;
    char          name[TC_IOT_LIGHT_NAME_LEN];
    unsigned int  color;
    float         brightness; // percentage 0.00 ~ 100.00
}tc_iot_demo_light;

#endif /* end of include guard */
