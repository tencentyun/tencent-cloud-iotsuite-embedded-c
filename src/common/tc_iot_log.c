#include "tc_iot_inc.h"

static char g_tc_iot_log_summary_print_str[128];

static tc_iot_log_level_e g_tc_iot_log_level = TC_IOT_LOG_LEVEL_TRACE;

void tc_iot_set_log_level(tc_iot_log_level_e log_level) {
    g_tc_iot_log_level = log_level;
}

tc_iot_log_level_e tc_iot_get_log_level(void) {
    return g_tc_iot_log_level ;
}

char tc_iot_log_level_enabled(tc_iot_log_level_e log_level) {
    return log_level >= g_tc_iot_log_level;
}

const char * tc_iot_log_summary_string(const char * src, int src_len) {
    int print_buf_len = sizeof(g_tc_iot_log_summary_print_str);

    if (src_len >= print_buf_len) {
        strncpy(g_tc_iot_log_summary_print_str, src, print_buf_len);
        strcpy(&g_tc_iot_log_summary_print_str[0]+print_buf_len-5, "...");
    } else {
        strncpy(&g_tc_iot_log_summary_print_str[0], src, src_len);
        g_tc_iot_log_summary_print_str[src_len] = '\0';
    }

    return &g_tc_iot_log_summary_print_str[0];
}
