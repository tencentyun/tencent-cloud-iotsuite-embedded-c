#include "tc_iot_device_config.h"
#include <sys/stat.h>
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"


/* anis color control codes */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_256_FORMAT   "\x1b[38;5;%dm"

extern tc_iot_shadow_local_data g_tc_iot_device_local_data;
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
                "%s[  lighting  ]|[color:%s]|[brightness:%s]|[%s]\n" ANSI_COLOR_RESET,
                ansi_color,
                ansi_color_name,
                brightness_bar,
                light->name
                );
    } else {
        /* 灯处于关闭状态时的展示 */
        tc_iot_hal_printf( ANSI_COLOR_YELLOW "%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET,
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                ANSI_COLOR_YELLOW "[" "light is off" "]|[color:%s]|[brightness:%s]|[%s]\n" ANSI_COLOR_RESET ,
                ansi_color_name,
                brightness_bar,
                light->name
                );
    }
}

int power_usage_inited = false;
#define REPORT_DURATION  20
int previous_left_seconds = REPORT_DURATION;
tc_iot_timer power_usage_timer;

void light_power_usage_calc(tc_iot_shadow_client * c) {
    int left_seconds = 0;
    if (!power_usage_inited) {
        power_usage_inited = true;
        tc_iot_hal_timer_init(&power_usage_timer);
        tc_iot_hal_timer_countdown_second(&power_usage_timer, REPORT_DURATION);
        previous_left_seconds = REPORT_DURATION;
        return;
    }

    if (tc_iot_hal_timer_is_expired(&power_usage_timer)) {
        g_tc_iot_device_local_data.power += 0.00001;
        tc_iot_report_device_data(c);
        tc_iot_hal_timer_countdown_second(&power_usage_timer, REPORT_DURATION);
        previous_left_seconds = REPORT_DURATION;
    } else {
        left_seconds = tc_iot_hal_timer_left_ms(&power_usage_timer)/1000;
        if (left_seconds != previous_left_seconds) {
            tc_iot_hal_printf("|%d...", left_seconds+1);
            if (left_seconds == 0) {
                tc_iot_hal_printf("\n");
            }
            previous_left_seconds = left_seconds;
        }
    }
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
        light_power_usage_calc(tc_iot_get_shadow_client());
    }

    tc_iot_server_destroy(tc_iot_get_shadow_client());
    return 0;
}

