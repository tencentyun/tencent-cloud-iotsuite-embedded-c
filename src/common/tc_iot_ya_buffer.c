#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_yabuffer_init(tc_iot_yabuffer_t *yabuffer, char *buf, int len) {
    IF_NULL_RETURN(yabuffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buf, TC_IOT_NULL_POINTER);
    IF_EQUAL_RETURN(len, 0, TC_IOT_INVALID_PARAMETER);
    yabuffer->data = buf;
    yabuffer->pos = 0;
    yabuffer->len = len;
    return TC_IOT_SUCCESS;
}

int tc_iot_yabuffer_reset(tc_iot_yabuffer_t *yabuffer) {
    IF_NULL_RETURN(yabuffer, TC_IOT_NULL_POINTER);
    yabuffer->pos = 0;
    if (yabuffer->data && yabuffer->len) {
        yabuffer->data[0] = 0;
    }
    return TC_IOT_SUCCESS;
}

int tc_iot_yabuffer_left(tc_iot_yabuffer_t *yabuffer) {
    int space_left;
    IF_NULL_RETURN(yabuffer, TC_IOT_NULL_POINTER);
    space_left = yabuffer->len - yabuffer->pos;
    return space_left;
}

char *tc_iot_yabuffer_current(tc_iot_yabuffer_t *yabuffer) {
    IF_NULL_RETURN_DATA(yabuffer, NULL);
    return yabuffer->data + yabuffer->pos;
}

int tc_iot_yabuffer_forward(tc_iot_yabuffer_t *yabuffer, int forward_len) {
    IF_NULL_RETURN(yabuffer, TC_IOT_NULL_POINTER);
    /* 检查是否超过最大限制 */
    IF_LESS_RETURN(yabuffer->len, yabuffer->pos + forward_len,
                   TC_IOT_INVALID_PARAMETER);
    /* 检查是否会导致负溢出 */
    IF_LESS_RETURN(yabuffer->pos + forward_len, 0,
                   TC_IOT_INVALID_PARAMETER);
    return yabuffer->pos += forward_len;
}

int tc_iot_yabuffer_append(tc_iot_yabuffer_t *yabuffer, const char *input) {
    IF_NULL_RETURN(yabuffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(input, TC_IOT_NULL_POINTER);
    return tc_iot_yabuffer_n_append(yabuffer, input, strlen(input));
}

int tc_iot_yabuffer_n_append(tc_iot_yabuffer_t *yabuffer, const char *input,
                             int len) {
    int copy_len;
    int space_left;

    IF_NULL_RETURN(yabuffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(input, TC_IOT_NULL_POINTER);

    space_left = tc_iot_yabuffer_left(yabuffer);
    IF_LESS_RETURN(space_left, len, TC_IOT_BUFFER_OVERFLOW);

    copy_len = space_left > len ? len : space_left;

    memcpy((void *)(yabuffer->data + yabuffer->pos), (void *)input,
           (size_t)copy_len);
    yabuffer->pos += copy_len;
    return copy_len;
}

#ifdef __cplusplus
}
#endif
