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

struct _tc_iot_timer { struct timeval end_time; } ;

#ifdef ENABLE_TLS
/* mbetls begin */
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time time
#define mbedtls_time_t time_t
#define mbedtls_fprintf fprintf
#define mbedtls_printf printf
#endif

#include "mbedtls/certs.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"

#include <string.h>
/* mbetls end */
#endif /* ifdef ENABLE_TLS */

#endif /* end of include guard */
