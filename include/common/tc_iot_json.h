#ifndef IOT_JSON_01121520_H
#define IOT_JSON_01121520_H

#include "tc_iot_inc.h"

int tc_iot_jsoneq(const char *json, jsmntok_t *tok, const char *s);
int tc_iot_jsoneq_len(const char *json, const jsmntok_t *tok, const char *s,
                      int len);
int tc_iot_json_find_token(const char *json, const jsmntok_t *root_token,
                           int count, const char *path, char *result,
                           int result_len);

#endif /* end of include guard */
