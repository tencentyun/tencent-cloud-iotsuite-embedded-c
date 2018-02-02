#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

long tc_iot_hal_timestamp(void* zone) {
    LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_sleep_ms(long sleep_ms) { 
    LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

long tc_iot_hal_random() {
    LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

void tc_iot_hal_srandom(unsigned int seed) { 
    LOG_ERROR("not implemented");  
}

#ifdef __cplusplus
}
#endif
