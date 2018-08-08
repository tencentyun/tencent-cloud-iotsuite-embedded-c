#include "tc_iot_inc.h"

char * tc_iot_util_byte_to_hex( const unsigned char * bytes, int bytes_len, char * output, int max_output_len)
{
    int i = 0;
    unsigned char temp;
    unsigned char high;
    unsigned char low;
    static const char map_byte_to_hex[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    if (output == NULL) {
        TC_IOT_LOG_ERROR("output is NULL");
        return NULL;
    }

    output[0] = '\0';

    if (bytes == NULL) {
        TC_IOT_LOG_ERROR("bytes is NULL");
        return output;
    }

    if ((max_output_len-1) < (bytes_len*2)) {
        TC_IOT_LOG_ERROR("max_output_len=%d not enought (bytes_len=%d)", max_output_len, bytes_len);
        return output;
    }

    for (i = 0; i < bytes_len; i++) {
        temp = bytes[i];
        high = (temp >> 4) & 0xF;
        output[2*i] = map_byte_to_hex[high];
        low  = temp & 0xF;
        output[2*i+1] = map_byte_to_hex[low];
    }
    output[2*bytes_len] = '\0';
    return output;
}

static int _tc_iot_get_char_value(char chr) {
    if (chr >= '0' && chr <= '9') {
        return chr - '0';
    }

    if (chr >= 'A' && chr <= 'F') {
        return chr - 'A';
    }

    if (chr >= 'a' && chr <= 'f') {
        return chr - 'a';
    }
    
    TC_IOT_LOG_ERROR("invalid char:%c", chr);
    return -1;
}

int tc_iot_util_hex_to_byte( const char * bytes, char * output, int max_output_len)
{
    int i = 0;
    int bytes_len = 0;
    unsigned char high;
    unsigned char low;

    if (output == NULL) {
        TC_IOT_LOG_ERROR("output is NULL");
        return 0;
    }

    output[0] = '\0';

    if (bytes == NULL) {
        TC_IOT_LOG_ERROR("bytes is NULL");
        return 0;
    }

    bytes_len = strlen(bytes);
    if ((bytes_len%2) != 0) {
        TC_IOT_LOG_ERROR("bytes_len=%d is invalid.", bytes_len);
        return 0;
    }

    if ((max_output_len-1) < (bytes_len/2)) {
        TC_IOT_LOG_ERROR("max_output_len=%d not enought (bytes_len=%d)", max_output_len, bytes_len);
        return 0;
    }

    for (i = 0; i < bytes_len/2; i++) {
        high = _tc_iot_get_char_value(bytes[2*i]);
        low  = _tc_iot_get_char_value(bytes[2*i+1]);
        if (high < 0 || low < 0) {
            return 0;
        }
        output[i] = ((high&0xF) << 4) | (low & 0xF);
    }
    output[bytes_len/2] = '\0';
    return bytes_len/2;
}

