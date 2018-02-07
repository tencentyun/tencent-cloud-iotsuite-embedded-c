#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

const static char encoding_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
const static char decoding_table[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 0,  0,  0,  0,  63, 0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

const static int mod_table[] = {0, 2, 1};

int tc_iot_base64_encode(const unsigned char *data, int input_length,
                     char *output_data, int max_output_len) {
    int i;
    int j;
    uint32_t octet_a;
    uint32_t octet_b;
    uint32_t octet_c;
    uint32_t triple;

    int output_length = 4 * ((input_length + 2) / 3);

    IF_NULL_RETURN(data, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(output_data, TC_IOT_NULL_POINTER);
    IF_LESS_RETURN(max_output_len, output_length, TC_IOT_INVALID_PARAMETER);

    for (i = 0, j = 0; i < input_length;) {
        octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        output_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        output_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        output_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        output_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (i = 0; i < mod_table[input_length % 3]; i++) {
        output_data[output_length - 1 - i] = '=';
    }

    return output_length;
}

int tc_iot_base64_decode(const char *data, int input_length,
                         unsigned char *output_data, int max_output_len) {
    int output_length = input_length / 4 * 3;
    int i = 0;
    int j = 0;
    uint32_t extet_a;
    uint32_t extet_b;
    uint32_t extet_c;
    uint32_t extet_d;
    uint32_t triple;

    IF_NULL_RETURN(data, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(output_data, TC_IOT_NULL_POINTER);
    IF_NOT_EQUAL_RETURN(input_length % 4, 0, TC_IOT_INVALID_PARAMETER);

    /* check if last 2 chars are '=' */
    for (i = 0; i < 2; i++) {
        if (data[input_length - 1 - i] == '=') {
            output_length--;
        }
    }

    IF_LESS_RETURN(max_output_len, output_length, TC_IOT_INVALID_PARAMETER);

    for (i = 0, j = 0; i < input_length;) {
        extet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        extet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        extet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        extet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        triple = (extet_a << 3 * 6) + (extet_b << 2 * 6) +
                          (extet_c << 1 * 6) + (extet_d << 0 * 6);

        if (j < output_length) {
            output_data[j++] = (triple >> 2 * 8) & 0xFF;
        }
        if (j < output_length) {
            output_data[j++] = (triple >> 1 * 8) & 0xFF;
        }
        if (j < output_length) {
            output_data[j++] = (triple >> 0 * 8) & 0xFF;
        }
    }
    if (output_length < max_output_len - 1) {
        output_data[output_length] = '\0';
    }

    return output_length;
}

#ifdef __cplusplus
}
#endif
