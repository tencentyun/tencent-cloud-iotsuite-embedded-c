#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client,  void * context);
void operate_device(tc_iot_shadow_local_data * device);

#define DECLARE_PROPERTY_DEF(name, type) {#name, TC_IOT_PROP_ ## name, type}

/* 设备本地数据类型及地址、回调函数等相关定义 */
tc_iot_shadow_property_def g_tc_iot_shadow_property_defs[] = {
    { "device_switch", TC_IOT_PROP_device_switch, TC_IOT_SHADOW_TYPE_BOOL},
    { "color", TC_IOT_PROP_color, TC_IOT_SHADOW_TYPE_ENUM},
    { "brightness", TC_IOT_PROP_brightness, TC_IOT_SHADOW_TYPE_NUMBER},
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
    TC_IOT_PROPTOTAL,
    &g_tc_iot_shadow_property_defs[0],
    _tc_iot_shadow_property_control_callback,
};


/* 设备状态数据 */
static tc_iot_shadow_local_data g_tc_iot_device_local_data = {
    false,
    TC_IOT_PROP_color_red,
    0,
};


static int _tc_iot_property_change( int property_id, void * data) {
    tc_iot_shadow_bool device_switch;
    tc_iot_shadow_enum color;
    tc_iot_shadow_number brightness;
    switch (property_id) {
        case TC_IOT_PROP_device_switch:
            device_switch = *(tc_iot_shadow_bool *)data;
            g_tc_iot_device_local_data.device_switch = device_switch;
            if (device_switch) {
                LOG_TRACE("do something for device_switch on");
            } else {
                LOG_TRACE("do something for device_switch off");
            }
            break;
        case TC_IOT_PROP_color:
            color = *(tc_iot_shadow_enum *)data;
            g_tc_iot_device_local_data.color = color;
            switch(color){
                case TC_IOT_PROP_color_red:
                    LOG_TRACE("do something for color = red");
                    break;
                case TC_IOT_PROP_color_green:
                    LOG_TRACE("do something for color = green");
                    break;
                case TC_IOT_PROP_color_blue:
                    LOG_TRACE("do something for color = blue");
                    break;
                default:
                    break;
            }
            break;
        case TC_IOT_PROP_brightness:
            brightness = *(tc_iot_shadow_number *)data;
            g_tc_iot_device_local_data.brightness = brightness;
            LOG_TRACE("do something for brightness=%d", brightness);
            break;
        default:
            LOG_WARN("unkown property id = %d", property_id);
            return TC_IOT_FAILURE;
    }

    tc_iot_report_propeties( 1, property_id, data);
    LOG_TRACE("operating device");
    operate_device(&g_tc_iot_device_local_data);
    return TC_IOT_SUCCESS;

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

