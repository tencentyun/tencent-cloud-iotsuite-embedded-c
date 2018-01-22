#ifndef IOT_URL_01091046_H
#define IOT_URL_01091046_H

int tc_iot_url_encode(const unsigned char* input, int input_len, char* output,
                      int max_output_len);
int tc_iot_url_decode(const unsigned char* input, int input_len, char* output,
                      int max_output_len);

#define HTTP_DEFAULT_PORT 80
#define HTTPS_DEFAULT_PORT 443

#define HTTP_SCHEME_PREFIX "http"
#define HTTP_SCHEME_PREFIX_LEN (sizeof(HTTP_SCHEME_PREFIX) - 1)
#define HTTPS_SCHEME_PREFIX "https"
#define HTTPS_SCHEME_PREFIX_LEN (sizeof(HTTPS_SCHEME_PREFIX) - 1)
#define SCHEME_SPLIT_STR "://"
#define SCHEME_SPLIT_STR_LEN (sizeof(SCHEME_SPLIT_STR) - 1)

/* length of: http://a */
#define URL_LEAST_LEN (HTTP_SCHEME_PREFIX_LEN + HTTPS_SCHEME_PREFIX_LEN + 1)

typedef struct _tc_iot_url_parse_result_t {
    int scheme_len;
    int host_start;
    int host_len;
    uint16_t port;
    char over_tls;
    int path_start;
    int path_len;
} tc_iot_url_parse_result_t;

int tc_iot_url_parse(const char* input_url, int input_url_len,
                     tc_iot_url_parse_result_t* result);

#endif /* end of include guard */
