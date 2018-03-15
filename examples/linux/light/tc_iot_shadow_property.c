#include "tc_iot_device_config.h"
#include "tc_iot_shadow_local_data.h"
#include "tc_iot_export.h"


void _device_on_message_received(tc_iot_message_data* md);
int _tc_iot_sync_shadow_property(tc_iot_shadow_property_def properties[], bool reported, const char * doc_start, jsmntok_t * json_token, int tok_count);
bool tc_iot_device_sync_reported(tc_iot_shadow_property_def properties[]);

void operate_device(tc_iot_shadow_local_data * device);

/* 设备状态数据 */
static tc_iot_shadow_control g_device_vars = 
{
    /* 当前最新同步的数据状态  */
    {
        false,  /* 开关状态 */
        "colorful device", /* 设备标识 */
        2, /* 设备光颜色控制 */
        100, /* 亮度 */
    },
    /* 已上报的数据状态 */
    {
        false,  /* 开关状态 */
        "", /* 设备标识 */
        0, /* 设备光颜色控制 */
        0, /* 亮度 */
    }
} ;

/* 设备本地数据类型及地址、回调函数等相关定义 */
tc_iot_shadow_property_def g_device_property_defs[] = {
    DECLARE_PROPERTY_DEF(g_device_vars.current.device_switch, 
            g_device_vars.reported.device_switch, device_switch, TC_IOT_SHADOW_TYPE_BOOL, _tc_iot_shadow_property_control_callback),
    DECLARE_PROPERTY_DEF(g_device_vars.current.name, 
            g_device_vars.
            reported.name, name, TC_IOT_SHADOW_TYPE_STRING, _tc_iot_shadow_property_control_callback),
    DECLARE_PROPERTY_DEF(g_device_vars.current.color, 
            g_device_vars.reported.color, color, TC_IOT_SHADOW_TYPE_NUMBER, _tc_iot_shadow_property_control_callback),
    DECLARE_PROPERTY_DEF(g_device_vars.current.brightness, 
            g_device_vars.reported.brightness, brightness, TC_IOT_SHADOW_TYPE_NUMBER, _tc_iot_shadow_property_control_callback),
};

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, const char * src,  void * context) {
    tc_iot_shadow_property_def * p_property = NULL;

    if (!msg) {
        LOG_ERROR("msg is null.");
        return TC_IOT_FAILURE;
    }

    if (msg->event == TC_IOT_SHADOW_EVENT_SERVER_CONTROL) {
        p_property = (tc_iot_shadow_property_def *)msg->data;
        if (!p_property) {
            LOG_ERROR("p_property is null.");
            return TC_IOT_FAILURE;
        }
        switch (p_property->id) {
            case TC_IOT_PROP_device_switch:
                LOG_TRACE("do something for device_switch change");
                break;
            case TC_IOT_PROP_name:
                LOG_TRACE("do something for name change");
                break;
            case TC_IOT_PROP_color:
                LOG_TRACE("do something for color change");
                break;
            case TC_IOT_PROP_brightness:
                LOG_TRACE("do something for brightness change");
                break;
            default:
                LOG_WARN("unkown property id = %d", p_property->id);
                return TC_IOT_SUCCESS;
        }
        LOG_TRACE("operating device");
        operate_device(&g_device_vars.current);
    } else if (msg->event == TC_IOT_SHADOW_EVENT_REPORTED_STATUS_SYNC) {
        p_property = (tc_iot_shadow_property_def *)msg->data;
        if (!p_property) {
            LOG_ERROR("p_property is null.");
            return TC_IOT_FAILURE;
        }
        switch (p_property->id) {
            case TC_IOT_PROP_device_switch:
                LOG_TRACE("device_switch reported status synchronized with server");
                break;
            case TC_IOT_PROP_name:
                LOG_TRACE("name reported status synchronized with server");
                break;
            case TC_IOT_PROP_color:
                LOG_TRACE("color reported status synchronized with server");
                break;
            case TC_IOT_PROP_brightness:
                LOG_TRACE("brightness reported status synchronized with server");
                break;
            default:
                LOG_WARN("unkown property id = %d", p_property->id);
                return TC_IOT_SUCCESS;
        }
    } else {
        LOG_TRACE("unkown event received, event=%d,src=%s", msg->event, src?src:"unknown");
    }
    return TC_IOT_SUCCESS;
}

