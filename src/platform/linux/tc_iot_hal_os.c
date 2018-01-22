#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

long tc_iot_hal_timestamp(void* zone) {
    return time(NULL);
}

#ifdef __cplusplus
}
#endif
