#ifndef TC_IOT_PLATFORM_H
#define TC_IOT_PLATFORM_H

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define tc_iot_hal_malloc malloc
#define tc_iot_hal_free free
#define tc_iot_hal_printf printf
#define tc_iot_hal_snprintf snprintf

typedef struct tc_iot_timer { struct timeval end_time; } tc_iot_timer;


#endif /* end of include guard */
