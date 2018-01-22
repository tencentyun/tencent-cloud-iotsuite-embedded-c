#ifndef IOT_BASE64_01091047_H
#define IOT_BASE64_01091047_H

int tc_base64_encode(const unsigned char *input, int input_length, char *output,
                     int max_output_len);
int tc_iot_base64_decode(const char *input, int input_length,
                         unsigned char *output, int max_output_len);

#endif /* end of include guard */
