#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

void tc_iot_hal_timer_init(tc_iot_timer* timer) {

    TC_IOT_LOG_ERROR("not implemented");  
}

char tc_iot_hal_timer_is_expired(tc_iot_timer* timer) {
    TC_IOT_LOG_ERROR("not implemented");  
    return 1;
}

void tc_iot_hal_timer_countdown_ms(tc_iot_timer* timer, unsigned int timeout) {
    TC_IOT_LOG_ERROR("not implemented");  
}

void tc_iot_hal_timer_countdown_second(tc_iot_timer* timer,
                                       unsigned int timeout) {
    TC_IOT_LOG_ERROR("not implemented");  
}

int tc_iot_hal_timer_left_ms(tc_iot_timer* timer) {
    TC_IOT_LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

#ifdef __cplusplus
}
#endif
