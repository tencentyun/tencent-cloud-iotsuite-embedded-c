#ifndef SYS_MEM_01091541_H
#define SYS_MEM_01091541_H

#include "tc_iot_inc.h"

void *tc_iot_hal_malloc(size_t size);
void tc_iot_hal_free(void *ptr);

int tc_iot_hal_printf(const char *format, ...);
int tc_iot_hal_snprintf(char *str, size_t size, const char *format, ...);
long tc_iot_hal_timestamp(void *);
int tc_iot_hal_sleep_ms(long sleep_ms);

long int tc_iot_hal_random(void);

void tc_iot_hal_srandom(unsigned int seed);

#endif /* end of include guard */
