#ifndef TC_IOT_OTA_DOWNLOAD
#define TC_IOT_OTA_DOWNLOAD

typedef int (*tc_iot_http_download_callback)(const void * context, const char * data, int data_len, int offset, int total);

typedef enum _tc_iot_http_response_parse_state {
    _PARSER_START,
    _PARSER_VERSION,
    _PARSER_IGNORE_TO_RETURN_CHAR,
    _PARSER_SKIP_NEWLINE_CHAR,
    _PARSER_HEADER,
    _PARSER_IGNORE_TO_BODY_START,
    _PARSER_END,
} tc_iot_http_response_parse_state;

typedef struct _tc_iot_http_response_parser {
    tc_iot_http_response_parse_state state;
    char  version;
    short status_code;
    int  content_length;
    const char * location;
    // const char * body;
}tc_iot_http_response_parser;

void tc_iot_http_parser_init(tc_iot_http_response_parser * parser);
int tc_iot_http_parser_analysis(tc_iot_http_response_parser * parser, const char * buffer, int buffer_len);

int tc_iot_ota_download(const char* api_url, int partial_start, tc_iot_http_download_callback download_callback, const void * context);
int tc_iot_ota_request_content_length(const char* api_url);

#endif /* end of include guard */
