#include "tc_iot_inc.h"

int tc_iot_coap_write_char(unsigned char * buffer, int buffer_len, unsigned char val) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER:
    }

    if (buffer_len < sizeof(val)) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }
    *(unsigned char *)buffer = val;
    return sizeof(val);
}

int tc_iot_coap_write_short(unsigned char * buffer, int buffer_len, unsigned short val) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER:
    }

    if (buffer_len < sizeof(val)) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }
    *(unsigned short *)buffer = htons(val);
    return sizeof(val);
}

int tc_iot_coap_write_int(unsigned char * buffer, int buffer_len, unsigned int val) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER:
    }

    if (buffer_len < sizeof(val)) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    *(unsigned int *)buffer = htonl(val);
    return sizeof(val);
}

int tc_iot_coap_write_int(unsigned char * buffer, int buffer_len, unsigned int val) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER:
    }

    if (buffer_len < sizeof(val)) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    *(unsigned int *)buffer = htonl(val);
    return sizeof(val);
}

int tc_iot_coap_write_bytes(unsigned char * buffer, int buffer_len, const unsigned char * bytes, int bytes_len) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER:
    }

    if (buffer_len < bytes_len) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    memcpy(buffer, bytes, bytes_len);
    return bytes_len;
}

int tc_iot_coap_serialize(unsigned char * buffer, int buffer_len, const tc_iot_coap_message * message) {
    int pos = 0;
    int ret = 0;

    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER:
    }

    if (NULL == message){
        TC_IOT_LOG_ERROR("message is null");
        return TC_IOT_NULL_POINTER:
    }

    ret = tc_iot_coap_write_char(buffer+pos, buffer_len+pos, message->header.all);
    if (ret < 0) {
        return ret;
    }
    pos += ret;

    ret = tc_iot_coap_write_char(buffer+pos, buffer_len+pos, message->code);
    if (ret < 0) {
        return ret;
    }
    pos += ret;

    ret = tc_iot_coap_write_short(buffer+pos, buffer_len+pos, message->message_id);
    if (ret < 0) {
        return ret;
    }
    pos += ret;

    ret = tc_iot_coap_write_bytes(buffer+pos, buffer_len+pos, message->token, message->header.token_len);
    if (ret < 0) {
        return ret;
    }
    pos += ret;

    return pos;
}

