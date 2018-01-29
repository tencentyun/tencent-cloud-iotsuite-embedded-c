#include "tc_iot_inc.h"

static int g_tc_iot_log_level = 0;

void tc_iot_set_log_level(tc_iot_log_level_e log_level) {
    g_tc_iot_log_level = log_level;
}

tc_iot_log_level_e tc_iot_get_log_level() {
    return g_tc_iot_log_level ;
}

char tc_iot_log_level_enabled(tc_iot_log_level_e log_level) {
    return log_level >= g_tc_iot_log_level;
}
