#ifndef TC_IOT_PLATFORM_H
#define TC_IOT_PLATFORM_H

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define tc_iot_hal_malloc malloc
#define tc_iot_hal_free free
#define tc_iot_hal_printf printf
#define tc_iot_hal_snprintf snprintf

typedef struct tc_iot_timer { long end_time; } tc_iot_timer;


#endif /* end of include guard */
