#ifndef TC_IOT_DEVICE_LOGIC_H
#define TC_IOT_DEVICE_LOGIC_H

#include "tc_iot_inc.h"

/* shadow properties struct define */
typedef struct _tc_iot_shadow_local_data {
    tc_iot_shadow_bool device_switch; /* true means on, false means off. */
    tc_iot_shadow_enum color; /* see enum for color: TC_IOT_PROP_color_xxx ...*/
    tc_iot_shadow_number brightness; /* light brightness*/
}tc_iot_shadow_local_data;

/* shadow properties id */
#define TC_IOT_PROP_device_switch 0
#define TC_IOT_PROP_color 1
#define TC_IOT_PROP_brightness 2
#define TC_IOT_PROP_TOTAL 3


/* enum for color */
#define TC_IOT_PROP_color_red 0
#define TC_IOT_PROP_color_green 1
#define TC_IOT_PROP_color_blue 2



int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client,  void * context);

extern tc_iot_shadow_client g_tc_iot_shadow_client;
extern tc_iot_shadow_property_def g_device_property_defs[];
#define DECLARE_PROPERTY_DEF(name, type, callback) {#name, TC_IOT_PROP_ ## name, type, callback}

void _device_on_message_received(tc_iot_message_data* md);
int _tc_iot_sync_shadow_property(tc_iot_shadow_property_def * properties, const char * doc_start, jsmntok_t * json_token, int tok_count);
int tc_iot_shadow_update_reported_propeties(int property_count, ...);
int tc_iot_shadow_update_desired_propeties(int property_count, ...); 


#endif /* end of include guard */
