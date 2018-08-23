#include "tc_iot_device_config.h"
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
    { "device_switch", TC_IOT_PROP_device_switch, TC_IOT_SHADOW_TYPE_BOOL, offsetof(tc_iot_shadow_local_data, device_switch),TC_IOT_MEMBER_SIZE(tc_iot_shadow_local_data,device_switch) },
    { "color", TC_IOT_PROP_color, TC_IOT_SHADOW_TYPE_ENUM, offsetof(tc_iot_shadow_local_data, color),TC_IOT_MEMBER_SIZE(tc_iot_shadow_local_data,color) },
    { "brightness", TC_IOT_PROP_brightness, TC_IOT_SHADOW_TYPE_NUMBER, offsetof(tc_iot_shadow_local_data, brightness),TC_IOT_MEMBER_SIZE(tc_iot_shadow_local_data,brightness) },
    { "power", TC_IOT_PROP_power, TC_IOT_SHADOW_TYPE_NUMBER, offsetof(tc_iot_shadow_local_data, power),TC_IOT_MEMBER_SIZE(tc_iot_shadow_local_data,power) },
    { "name", TC_IOT_PROP_name, TC_IOT_SHADOW_TYPE_STRING, offsetof(tc_iot_shadow_local_data, name),TC_IOT_MEMBER_SIZE(tc_iot_shadow_local_data,name) },
};


/* 设备当前状态数据 */
tc_iot_shadow_local_data g_tc_iot_device_local_data = {
    false,
    TC_IOT_PROP_color_red,
    0,
    1,
    {'\0'},
};

/* 设备状态控制数据 */
static tc_iot_shadow_local_data g_tc_iot_device_desired_data = {
    false,
    TC_IOT_PROP_color_red,
    0,
    1,
    {'\0'},
};

/* 设备已上报状态数据 */
tc_iot_shadow_local_data g_tc_iot_device_reported_data = {
    false,
    TC_IOT_PROP_color_red,
    0,
    1,
    {'\0'},
};

/* 设备初始配置 */
tc_iot_shadow_config g_tc_iot_shadow_config = {
    {
        {
            /* device info*/
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
            TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
            TC_IOT_CONFIG_AUTH_MODE, TC_IOT_CONFIG_REGION, TC_IOT_CONFIG_AUTH_API_URL,
        },
        TC_IOT_CONFIG_MQ_SERVER_HOST,
        TC_IOT_CONFIG_MQ_SERVER_PORT,
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
    TC_IOT_SHADOW_SUB_TOPIC_DEF,
    TC_IOT_SHADOW_PUB_TOPIC_DEF,
    tc_iot_device_on_message_received,
    TC_IOT_PROPTOTAL,
    &g_tc_iot_shadow_property_defs[0],
    _tc_iot_shadow_property_control_callback,
    &g_tc_iot_device_local_data,
    &g_tc_iot_device_reported_data,
    &g_tc_iot_device_desired_data,
};


static int _tc_iot_property_change( int property_id, void * data) {
    tc_iot_shadow_bool device_switch;
    tc_iot_shadow_enum color;
    tc_iot_shadow_number brightness;
    tc_iot_shadow_number power;
    tc_iot_shadow_string name;
    switch (property_id) {
        case TC_IOT_PROP_device_switch:
            device_switch = *(tc_iot_shadow_bool *)data;
            g_tc_iot_device_local_data.device_switch = device_switch;
            if (device_switch) {
                TC_IOT_LOG_TRACE("do something for device_switch on");
            } else {
                TC_IOT_LOG_TRACE("do something for device_switch off");
            }
            break;
        case TC_IOT_PROP_color:
            color = *(tc_iot_shadow_enum *)data;
            g_tc_iot_device_local_data.color = color;
            switch(color){
                case TC_IOT_PROP_color_red:
                    TC_IOT_LOG_TRACE("do something for color = red");
                    break;
                case TC_IOT_PROP_color_green:
                    TC_IOT_LOG_TRACE("do something for color = green");
                    break;
                case TC_IOT_PROP_color_blue:
                    TC_IOT_LOG_TRACE("do something for color = blue");
                    break;
                default:
                    TC_IOT_LOG_WARN("do something for color = unknown");
                    /* 如果能正常处理未知状态，则返回 TC_IOT_SUCCESS */
                    /* 如果不能正常处理未知状态，则返回 TC_IOT_FAILURE */
                    return TC_IOT_FAILURE;
            }
            break;
        case TC_IOT_PROP_brightness:
            brightness = *(tc_iot_shadow_number *)data;
            g_tc_iot_device_local_data.brightness = brightness;
            TC_IOT_LOG_TRACE("do something for brightness=%f", brightness);
            break;
        case TC_IOT_PROP_power:
            power = *(tc_iot_shadow_number *)data;
            g_tc_iot_device_local_data.power = power;
            TC_IOT_LOG_TRACE("do something for power=%f", power);
            break;
        case TC_IOT_PROP_name:
            name = (char *)data;
            strcpy(g_tc_iot_device_local_data.name, name);
            TC_IOT_LOG_TRACE("do something for name=%s", name);
            break;
        default:
            TC_IOT_LOG_WARN("unkown property id = %d", property_id);
            return TC_IOT_FAILURE;
    }

    TC_IOT_LOG_TRACE("operating device");
    operate_device(&g_tc_iot_device_local_data);
    return TC_IOT_SUCCESS;

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
    } else {
        TC_IOT_LOG_TRACE("unkown event received, event=%d", msg->event);
    }
    return TC_IOT_SUCCESS;
}

