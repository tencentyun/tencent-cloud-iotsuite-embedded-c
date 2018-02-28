#include "tc_iot_inc.h"

static tc_iot_log_level_e g_tc_iot_log_level = TC_IOT_LOG_TRACE;

void tc_iot_set_log_level(tc_iot_log_level_e log_level) {
    g_tc_iot_log_level = log_level;
}

tc_iot_log_level_e tc_iot_get_log_level(void) {
    return g_tc_iot_log_level ;
}

char tc_iot_log_level_enabled(tc_iot_log_level_e log_level) {
    return log_level >= g_tc_iot_log_level;
}
