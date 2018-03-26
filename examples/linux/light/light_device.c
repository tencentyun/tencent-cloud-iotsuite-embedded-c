#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"


/* anis color control codes */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_256_FORMAT   "\x1b[38;5;%dm"

int run_shadow(tc_iot_shadow_config * p_client_config);
void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv) ;
void get_message_ack_callback(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context);
extern tc_iot_shadow_config g_tc_iot_shadow_config;

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
 * @brief operate_device 操作灯光控制开关
 *
 * @param light 灯状态数据
 */
void operate_device(tc_iot_shadow_local_data * light) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    const char * ansi_color = NULL;
    const char * ansi_color_name = NULL;
    char brightness_bar[]      = "||||||||||||||||||||";
    int brightness_bar_len = strlen(brightness_bar);
    int i = 0;

    switch(light->color) {
        case TC_IOT_PROP_color_red:
            ansi_color = ANSI_COLOR_RED;
            ansi_color_name = " RED ";
            break;
        case TC_IOT_PROP_color_green:
            ansi_color = ANSI_COLOR_GREEN;
            ansi_color_name = "GREEN";
            break;
        case TC_IOT_PROP_color_blue:
            ansi_color = ANSI_COLOR_BLUE;
            ansi_color_name = " BLUE";
            break;
        default:
            ansi_color = ANSI_COLOR_YELLOW;
            ansi_color_name = "UNKNOWN";
            break;
    }

    /* 灯光亮度显示条 */
    brightness_bar_len = light->brightness >= 100?brightness_bar_len:(int)((light->brightness * brightness_bar_len)/100);
    for (i = brightness_bar_len; i < strlen(brightness_bar); i++) {
        brightness_bar[i] = '-';
    }

    if (light->device_switch) {
        /* 灯光开启式，按照控制参数展示 */
        tc_iot_hal_printf( "%s%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET,
                ansi_color,
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                "%s[  lighting  ]|[color:%s]|[brightness:%s]\n" ANSI_COLOR_RESET,
                ansi_color,
                ansi_color_name,
                brightness_bar
                );
    } else {
        /* 灯处于关闭状态时的展示 */
        tc_iot_hal_printf( ANSI_COLOR_YELLOW "%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET,
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                ANSI_COLOR_YELLOW "[" "light is off" "]|[color:%s]|[brightness:%s]\n" ANSI_COLOR_RESET ,
                ansi_color_name,
                brightness_bar
                );
    }
}

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;
    bool token_defined;
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    p_client_config = &(g_tc_iot_shadow_config.mqtt_client_config);

    /* 解析命令行参数 */
    parse_command(p_client_config, argc, argv);

    /* 根据 product id 和device name 定义，生成发布和订阅的 Topic 名称。 */
    snprintf(g_tc_iot_shadow_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_tc_iot_shadow_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_PUB_TOPIC_FMT,
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    /* 判断是否需要获取动态 token */
    token_defined = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);

    if (!token_defined) {
        /* 获取动态 token */
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = http_refresh_auth_token_with_expire(
                TC_IOT_CONFIG_AUTH_API_URL, NULL,
                timestamp, nonce,
                &p_client_config->device_info, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return 0;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }

    ret = tc_iot_server_init(&g_tc_iot_shadow_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("tc_iot_server_init failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return 0;
    }

    /* tc_iot_report_firm(3, "mac","00-00-00-00-00", "sdk-ver", "1.0", "firm-ver","2.0.2018012.pre"); */

    while (!stop) {
        tc_iot_server_loop(200);
    }

    tc_iot_server_destroy();
    return 0;
}

