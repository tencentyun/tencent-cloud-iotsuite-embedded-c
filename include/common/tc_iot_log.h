#ifndef IOT_LOG_H
#define IOT_LOG_H

#include "tc_iot_inc.h"


/**
 * @brief 日志等级  TRACE < DEBUG < INFO < WARN < ERROR < CRIT < OFF
 */
typedef enum _tc_iot_log_level_e{
    TC_IOT_LOG_TRACE = 0,
    TC_IOT_LOG_DEBUG = 1,
    TC_IOT_LOG_INFO = 2,
    TC_IOT_LOG_WARN = 3,
    TC_IOT_LOG_ERROR = 4,
    TC_IOT_LOG_CRIT = 5,
    TC_IOT_LOG_OFF  = 6,
} tc_iot_log_level_e;

void tc_iot_set_log_level(tc_iot_log_level_e log_level);
tc_iot_log_level_e tc_iot_get_log_level();
char tc_iot_log_level_enabled(tc_iot_log_level_e log_level);

#ifdef ENABLE_LOG_TRACE
#define LOG_TRACE(...)                                             \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_TRACE)){               \
        tc_iot_hal_printf("TRACE %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                            \
        tc_iot_hal_printf("\n");                                   \
    }

#define IOT_FUNC_ENTRY \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_TRACE)){               \
     tc_iot_hal_printf("IOT_FUNC_ENTRY %s:%d \n", __FUNCTION__, __LINE__); \
    }

#define IOT_FUNC_EXIT \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_TRACE)){               \
     tc_iot_hal_printf("IOT_FUNC_EXIT %s:%d \n", __FUNCTION__, __LINE__); \
    }

#define IOT_FUNC_EXIT_RC(x)                                                   \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_TRACE)){               \
        tc_iot_hal_printf("IOT_FUNC_EXIT %s:%d Return : %d \n", __FUNCTION__, \
                          __LINE__, x);                                       \
    }                                                                     \
    return x;                                                             \

#else
#define LOG_TRACE(...)
#define IOT_FUNC_ENTRY
#define IOT_FUNC_EXIT
#define IOT_FUNC_EXIT_RC(x) \
    { return x; }
#endif

#ifdef ENABLE_LOG_DEBUG
#define LOG_DEBUG(...)                                             \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_DEBUG)){               \
        tc_iot_hal_printf("DEBUG %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                            \
        tc_iot_hal_printf("\n");                                   \
    }

#else
#define LOG_DEBUG(...)
#endif

#ifdef ENABLE_LOG_INFO
#define LOG_INFO(...)                   \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_INFO)){   \
        tc_iot_hal_printf("INFO %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__); \
        tc_iot_hal_printf("\n");        \
    }

#else
#define LOG_INFO(...)
#endif

#ifdef ENABLE_LOG_WARN
#define LOG_WARN(...)                                             \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_WARN)){               \
        tc_iot_hal_printf("WARN %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                           \
        tc_iot_hal_printf("\n");                                  \
    }

#else
#define LOG_WARN(...)
#endif

#ifdef ENABLE_LOG_ERROR
#define LOG_ERROR(...)                                             \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_ERROR)){               \
        tc_iot_hal_printf("ERROR %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                            \
        tc_iot_hal_printf("\n");                                   \
    }

#else
#define LOG_ERROR(...)
#endif

#ifdef ENABLE_LOG_CRIT
#define LOG_CRIT(...)                                             \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_CRIT)){               \
        tc_iot_hal_printf("CRIT %s:%d ", __FUNCTION__, __LINE__); \
        tc_iot_hal_printf(__VA_ARGS__);                           \
        tc_iot_hal_printf("\n");                                  \
    }

#else
#define LOG_CRIT(...)
#endif

#endif /* end of include guard */
