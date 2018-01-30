#ifndef TC_IOT_HAL_TIMER_H
#define TC_IOT_HAL_TIMER_H

#include "tc_iot_inc.h"

typedef struct tc_iot_timer tc_iot_timer;


/**
 * @brief tc_iot_hal_timer_init 初始化或重置定时器
 *
 * @param  p_timer 定时器指针
 */
void tc_iot_hal_timer_init(tc_iot_timer* p_timer);


/**
 * @brief tc_iot_hal_timer_is_expired 判断定时器是否已经过期
 *
 * @param p_timer 定时器指针
 *
 * @return 
 */
char tc_iot_hal_timer_is_expired(tc_iot_timer* p_timer);

/**
 * @brief tc_iot_hal_timer_countdown_ms 设定定时器时延，单位：ms
 *
 * @param p_timer 定时器指针
 * @param delay_ms 设定延时毫秒数
 */
void tc_iot_hal_timer_countdown_ms(tc_iot_timer* p_timer, unsigned int delay_ms);

/**
 * @brief tc_iot_hal_timer_countdown_second 设定定时器时延，单位：s
 *
 * @param p_timer 定时器指针 
 * @param delay_sec 设定延时秒数
 */
void tc_iot_hal_timer_countdown_second(tc_iot_timer* p_timer, unsigned int delay_sec);


/**
 * @brief tc_iot_hal_timer_left_ms 检查定时器剩余时长，单位：ms
 *
 * @param p_timer 定时器指针 
 *
 * @return 当前剩余毫秒数，小于或等于0时，表示定时器已经超时
 */
int tc_iot_hal_timer_left_ms(tc_iot_timer* p_timer);

#endif /* end of include guard */
