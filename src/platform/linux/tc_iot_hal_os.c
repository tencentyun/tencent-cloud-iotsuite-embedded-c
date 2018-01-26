#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

long tc_iot_hal_timestamp(void* zone) {
    // TODO remove test
    return 1999;
    return time(NULL);
}

int tc_iot_hal_sleep_ms(long sleep_ms){
    return usleep(sleep_ms*1000);
}

long tc_iot_hal_random()
{
    // TODO remove test
    return 1999;
    return random();
}

void tc_iot_hal_srandom(unsigned int seed)
{
    return srand(seed);
}

#ifdef __cplusplus
}
#endif
