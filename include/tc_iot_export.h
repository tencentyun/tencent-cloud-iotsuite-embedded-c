#ifndef IOT_EXPORT_H
#define IOT_EXPORT_H

#include "tc_iot_inc.h"

int tc_iot_shadow_construct(tc_iot_shadow_client *,
                            tc_iot_shadow_config *p_config);
void tc_iot_shadow_destroy(tc_iot_shadow_client *p_shadow_client);

char tc_iot_shadow_isconnected(tc_iot_shadow_client *p_shadow_client);
int tc_iot_shadow_yield(tc_iot_shadow_client *p_shadow_client, int timeout_ms);
int tc_iot_shadow_get(tc_iot_shadow_client *p_shadow_client);
int tc_iot_shadow_update(tc_iot_shadow_client *p_shadow_client, char *pJsonDoc);
int tc_iot_shadow_delete(tc_iot_shadow_client *p_shadow_client, char *pJsonDoc);

#endif /* end of include guard */
