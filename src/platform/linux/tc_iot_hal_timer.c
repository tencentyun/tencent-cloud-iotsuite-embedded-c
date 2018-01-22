#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

void tc_iot_hal_timer_init(tc_iot_timer* timer) {
    timer->end_time = (struct timeval){0, 0};
}

char tc_iot_hal_timer_is_expired(tc_iot_timer* timer) {
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->end_time, &now, &res);
    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}

void tc_iot_hal_timer_countdown_ms(tc_iot_timer* timer, unsigned int timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
    timeradd(&now, &interval, &timer->end_time);
}

void tc_iot_hal_timer_countdown_second(tc_iot_timer* timer,
                                       unsigned int timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {timeout, 0};
    timeradd(&now, &interval, &timer->end_time);
}

int tc_iot_hal_timer_left_ms(tc_iot_timer* timer) {
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->end_time, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

#ifdef __cplusplus
}
#endif
