#ifndef IOT_YA_BUFFER_01091046_H
#define IOT_YA_BUFFER_01091046_H

typedef struct _tc_iot_yabuffer_t {
    char* data;
    int pos;
    int len;
} tc_iot_yabuffer_t;

int tc_iot_yabuffer_init(tc_iot_yabuffer_t* yabuffer, char* buf, int len);
int tc_iot_yabuffer_reset(tc_iot_yabuffer_t* yabuffer);
int tc_iot_yabuffer_left(tc_iot_yabuffer_t* buffer);
int tc_iot_yabuffer_append_format(tc_iot_yabuffer_t* yabuffer,
                                  const char* format, ...);
int tc_iot_yabuffer_n_append(tc_iot_yabuffer_t* yabuffer, const char* input,
                             int len);
int tc_iot_yabuffer_append(tc_iot_yabuffer_t* yabuffer, const char* input);
char* tc_iot_yabuffer_current(tc_iot_yabuffer_t* yabuffer);
int tc_iot_yabuffer_forward(tc_iot_yabuffer_t* yabuffer, int forward_len);

#endif /* end of include guard */
