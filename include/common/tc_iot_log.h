#ifndef IOT_LOG_H
#define IOT_LOG_H

#include "tc_iot_inc.h"

extern int g_iot_log_fd;
typedef enum _LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRIT,
} LogLevel;

#ifdef ENABLE_LOG_TRACE
#define LOG_TRACE(...)                                             \
    {                                                              \
        tc_iot_hal_printf("TRACE %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                            \
        tc_iot_hal_printf("\n");                                   \
    }

#define IOT_FUNC_ENTRY \
    { tc_iot_hal_printf("IOT_FUNC_ENTRY %s:%d \n", __FUNCTION__, __LINE__); }
#define IOT_FUNC_EXIT \
    { tc_iot_hal_printf("IOT_FUNC_EXIT %s:%d \n", __FUNCTION__, __LINE__); }
#define IOT_FUNC_EXIT_RC(x)                                                   \
    {                                                                         \
        tc_iot_hal_printf("IOT_FUNC_EXIT %s:%d Return : %d \n", __FUNCTION__, \
                          __LINE__, x);                                       \
        return x;                                                             \
    }
#else
#define LOG_TRACE(...)
#define IOT_FUNC_ENTRY
#define IOT_FUNC_EXIT
#define IOT_FUNC_EXIT_RC(x) \
    { return x; }
#endif

#ifdef ENABLE_LOG_DEBUG
#define LOG_DEBUG(...)                                             \
    {                                                              \
        tc_iot_hal_printf("DEBUG %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                            \
        tc_iot_hal_printf("\n");                                   \
    }
#else
#define LOG_DEBUG(...)
#endif

#ifdef ENABLE_LOG_INFO
#define LOG_INFO(...)                   \
    {                                   \
        tc_iot_hal_printf(__VA_ARGS__); \
        tc_iot_hal_printf("\n");        \
    }
#else
#define LOG_INFO(...)
#endif

#ifdef ENABLE_LOG_WARN
#define LOG_WARN(...)                                             \
    {                                                             \
        tc_iot_hal_printf("WARN %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                           \
        tc_iot_hal_printf("\n");                                  \
    }
#else
#define LOG_WARN(...)
#endif

#ifdef ENABLE_LOG_ERROR
#define LOG_ERROR(...)                                             \
    {                                                              \
        tc_iot_hal_printf("ERROR %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                            \
        tc_iot_hal_printf("\n");                                   \
    }
#else
#define LOG_ERROR(...)
#endif

#ifdef ENABLE_LOG_CRIT
#define LOG_CRIT(...)                                             \
    {                                                             \
        tc_iot_hal_printf("CRIT %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                           \
        tc_iot_hal_printf("\n");                                  \
    }
#else
#define LOG_CRIT(...)
#endif

#endif /* end of include guard */
