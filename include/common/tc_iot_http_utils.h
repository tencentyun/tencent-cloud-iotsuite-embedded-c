#ifndef IOT_HTTP_UTILS_01111941_H
#define IOT_HTTP_UTILS_01111941_H

#include "tc_iot_inc.h"

#define HTTP_VER_1_0 "1.0"
#define HTTP_VER_1_1 "1.1"

#define HTTP_PUT "GET"
#define HTTP_POST "POST"
#define HTTP_GET "GET"

#define HTTP_SPLIT_STR "\r\n"
#define HTTP_REQUEST_LINE_FMT ("%s %s HTTP/%s" HTTP_SPLIT_STR)
#define HTTP_HEADER_FMT "%s: %.*s\r\n"
#define HTTP_HEADER_HOST "Host"
#define HTTP_HEADER_ACCEPT "Accept"
#define HTTP_HEADER_ACCEPT_ENCODING "Accept-Encoding"
#define HTTP_HEADER_USER_AGENT "User-Agent"
#define HTTP_HEADER_CONTENT_LENGTH "Content-Length"
#define HTTP_HEADER_CONTENT_TYPE "Content-Type"
#define HTTP_CONTENT_FORM_URLENCODED "application/x-www-form-urlencoded"
#define HTTP_CONTENT_FORM_DATA "multipart/form-data"
#define HTTP_CONTENT_JSON "application/json"

#define HTTP_BODY_FMT "\r\n%s"

typedef struct _tc_iot_http_request {
    tc_iot_yabuffer_t buf;
} tc_iot_http_request;

int tc_iot_http_request_init(tc_iot_http_request* request, const char* method,
                             const char* abs_path, int abs_path_len,
                             const char* http_version);
int tc_iot_http_request_append_header(tc_iot_http_request* request,
                                      const char* header, const char* val);
int tc_iot_http_request_n_append_header(tc_iot_http_request* request,
                                        const char* header, const char* val,
                                        int val_len);
int tc_iot_http_request_append_body(tc_iot_http_request* request,
                                    const char* body);

int tc_iot_create_http_request(tc_iot_http_request* request, const char* host,
                               int host_len, const char* method,
                               const char* abs_path, int abs_path_len,
                               const char* http_version, const char* user_agent,
                               const char* content_type, const char* body);

int tc_iot_create_post_request(tc_iot_http_request* request,
                               const char* abs_path, int abs_path_len,
                               const char* host, int host_len,
                               const char* body);

int tc_iot_calc_auth_sign(char* sign_out, int max_sign_len, const char* secret,
                          int secret_len, const char* client_id,
                          int client_id_len, const char* device_name,
                          int device_name_len, long expire, long nonce,
                          const char* product_id, int product_id_len,
                          long timestamp);

int tc_iot_create_auth_request_form(char* form, int max_form_len,
                                    const char* secret, int secret_len,
                                    const char* client_id, int client_id_len,
                                    const char* device_name,
                                    int device_name_len, long expire,
                                    long nonce, const char* product_id,
                                    int product_id_len, long timestamp);

#endif /* end of include guard */
