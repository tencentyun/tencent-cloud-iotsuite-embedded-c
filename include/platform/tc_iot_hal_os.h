#ifndef TC_IOT_HAL_OS_H
#define TC_IOT_HAL_OS_H

#include "tc_iot_inc.h"


/**
 * @brief tc_iot_hal_malloc 分配所需的内存空间，并返回一个指向它的指针
 *
 * @param size 待分配空间的大小
 *
 * @return 指针指向分配到的内存空间，返回空指针则表示未分配到内存
 */
void *tc_iot_hal_malloc(size_t size);


/**
 * @brief tc_iot_hal_free 释放之前调用 tc_iot_hal_malloc 所分配的内存空间。 
 *
 * @param ptr 待释放的内存
 */
void tc_iot_hal_free(void *ptr);


/**
 * @brief tc_iot_hal_printf 发送格式化输出到标准输出 stdout
 *
 * @param format 格式化字符串
 * @param ... 格式化相关输入参数
 *
 * @return 输出字节数
 */
// int tc_iot_hal_printf(const char *format, ...);

/**
 * @brief tc_iot_hal_snprintf 发送格式化输出到字符串
 *
 * @param str 输出缓存区
 * @param size 输出缓存区大小
 * @param format 格式化字符串
 * @param ... 格式化相关输入参数
 *
 * @return 输出字节数，需要注意的是：当返回值与入参size大小一致时，有可能缓存区
 * 大小不够，内容并未完全输出
 */
int tc_iot_hal_snprintf(char *str, size_t size, const char *format, ...);


/**
 * @brief tc_iot_hal_timestamp 系统时间戳，格林威治时间 1970-1-1 00点起总秒数
 *
 * @param 保留参数，暂未启用
 *
 * @return 时间戳
 */
long tc_iot_hal_timestamp(void *);


/**
 * @brief tc_iot_hal_sleep_ms 睡眠挂起一定时长，单位：ms
 *
 * @param sleep_ms 时长，单位ms
 *
 * @return 0 表示成功，-1 表示失败
 */
int tc_iot_hal_sleep_ms(long sleep_ms);


/**
 * @brief tc_iot_hal_random 获取随机数
 *
 * @return 获得的随机数
 */
long tc_iot_hal_random(void);



/**
 * @brief tc_iot_hal_srandom 设置随机数种子值
 *
 * @param seed 随机数种子值
 */
void tc_iot_hal_srandom(unsigned int seed);

#endif /* end of include guard */
