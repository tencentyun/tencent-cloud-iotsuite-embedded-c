#include "tc_iot_inc.h"

static int g_tc_iot_log_level = 0;

<<<<<<< HEAD
void tc_iot_set_log_level(int log_level) { g_tc_iot_log_level = log_level; }

int tc_iot_get_log_level() { return g_tc_iot_log_level; }
=======
void tc_iot_set_log_level(tc_iot_log_level_e log_level) {
    g_tc_iot_log_level = log_level;
}

tc_iot_log_level_e tc_iot_get_log_level() {
    return g_tc_iot_log_level ;
}
>>>>>>> dc76d78ce0ac7817aa77d2de3d4f910124789795

char tc_iot_log_level_enabled(tc_iot_log_level_e log_level) {
    return log_level >= g_tc_iot_log_level;
}
