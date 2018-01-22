#ifndef SYS_MEM_01091541_H
#define SYS_MEM_01091541_H

#include "tc_iot_inc.h"

void *tc_iot_hal_malloc(size_t size);
void tc_iot_hal_free(void *ptr);

int tc_iot_hal_printf(const char *format, ...);
int tc_iot_hal_snprintf(char *str, size_t size, const char *format, ...);
long tc_iot_hal_timestamp(void *);

#endif /* end of include guard */
