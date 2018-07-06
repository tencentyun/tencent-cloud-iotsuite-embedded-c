#include "tc_iot_inc.h"

const char * tc_iot_util_byte_to_hex( const unsigned char * bytes, int bytes_len, char * output, int max_output_len)
{
    int i = 0;
    unsigned char temp;
    unsigned char high;
    unsigned char low;
    static const char map_byte_to_hex[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    if (bytes == NULL) {
        return "bytes is NULL";
    }

    if (output == NULL) {
        return "output is NULL";
    }

    if ((max_output_len-1) < (bytes_len*2)) {
        return "max_output_len not enough";
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

