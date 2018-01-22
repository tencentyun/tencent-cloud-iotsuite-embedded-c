#ifndef SYS_TIMER_01091118_H
#define SYS_TIMER_01091118_H

#include "tc_iot_inc.h"

typedef struct tc_iot_timer { struct timeval end_time; } tc_iot_timer;

void tc_iot_hal_timer_init(tc_iot_timer*);
char tc_iot_hal_timer_is_expired(tc_iot_timer*);
void tc_iot_hal_timer_countdown_ms(tc_iot_timer*, unsigned int);
void tc_iot_hal_timer_countdown_second(tc_iot_timer*, unsigned int);
int tc_iot_hal_timer_left_ms(tc_iot_timer*);

#endif /* end of include guard */
