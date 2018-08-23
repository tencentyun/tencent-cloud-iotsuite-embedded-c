#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"


void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv) ;
void get_message_ack_callback(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context);
extern tc_iot_shadow_config g_tc_iot_shadow_config;
extern tc_iot_shadow_local_data g_tc_iot_device_local_data;

/* 循环退出标识 */
volatile int stop = 0;
void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop ++;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop ++;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
    if (stop >= 3) {
        tc_iot_hal_printf("SIGINT/SIGTERM received over %d times, force shutdown now.\n", stop);
        exit(0);
    }
}


/**
 * @brief operate_device 操作设备控制开关
 *
 * @param p_device_data 设备状态数据
 */
void operate_device(tc_iot_shadow_local_data * p_device_data) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    tc_iot_hal_printf( "%04d-%02d-%02d %02d:%02d:%02d do something for data change.\n" ,
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}


/**
 * @brief 本函数演示，当设备端状态发生变化时，如何更新设备端数据，并上报给服务端。
 */
void do_sim_data_change(void) {
    TC_IOT_LOG_TRACE("simulate data change.");
    int i = 0;

    g_tc_iot_device_local_data.param_bool = !g_tc_iot_device_local_data.param_bool;

    g_tc_iot_device_local_data.param_enum += 1;
    g_tc_iot_device_local_data.param_enum %= 3;

    g_tc_iot_device_local_data.param_number += 1;
    g_tc_iot_device_local_data.param_number = g_tc_iot_device_local_data.param_number > 4095?0:g_tc_iot_device_local_data.param_number;

    for (i = 0; i < 0+1;i++) {
        g_tc_iot_device_local_data.param_string[i] += 1;
        g_tc_iot_device_local_data.param_string[i] = g_tc_iot_device_local_data.param_string[0] > 'Z'?'A':g_tc_iot_device_local_data.param_string[0];
        g_tc_iot_device_local_data.param_string[i] = g_tc_iot_device_local_data.param_string[0] < 'A'?'A':g_tc_iot_device_local_data.param_string[0];
    }
    g_tc_iot_device_local_data.param_string[0+2] = 0;


    /* 上报数据最新状态 */
    tc_iot_report_device_data(tc_iot_get_shadow_client());
}

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;
    bool use_static_token;
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    setbuf(stdout, NULL);

    p_client_config = &(g_tc_iot_shadow_config.mqtt_client_config);

    /* 解析命令行参数 */
    parse_command(p_client_config, argc, argv);

    /* 根据 product id 和device name 定义，生成发布和订阅的 Topic 名称。 */
    snprintf(g_tc_iot_shadow_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SHADOW_SUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_tc_iot_shadow_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SHADOW_PUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    /* 判断是否需要获取动态 token */
    use_static_token = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);

    if (!use_static_token) {
        /* 获取动态 token */
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = TC_IOT_AUTH_FUNC( timestamp, nonce, &p_client_config->device_info, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return 0;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }

    ret = tc_iot_server_init(tc_iot_get_shadow_client(), &g_tc_iot_shadow_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("tc_iot_server_init failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return 0;
    }

    while (!stop) {
        tc_iot_server_loop(tc_iot_get_shadow_client(), 200);
    }

    tc_iot_server_destroy(tc_iot_get_shadow_client());
    return 0;
}

