#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_http_request_init(tc_iot_http_request* request, const char* method,
                             const char* abs_path, int abs_path_len,
                             const char* http_version) {
    char* current = NULL;
    int buffer_left;
    int ret;

    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(abs_path, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(http_version, TC_IOT_NULL_POINTER);
    tc_iot_yabuffer_reset(&(request->buf));

    current = tc_iot_yabuffer_current(&(request->buf));
    buffer_left = tc_iot_yabuffer_left(&(request->buf));
    ret = tc_iot_hal_snprintf(current, buffer_left, HTTP_REQUEST_LINE_FMT,
                              method, abs_path, http_version);
    if (ret > 0) {
        tc_iot_yabuffer_forward(&(request->buf), ret);
    }
    return ret;
}

int tc_iot_http_request_append_header(tc_iot_http_request* request,
                                      const char* header, const char* val) {
    char* current;
    int buffer_left, ret;

    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(header, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(val, TC_IOT_NULL_POINTER);

    current = tc_iot_yabuffer_current(&(request->buf));
    buffer_left = tc_iot_yabuffer_left(&(request->buf));
    ret = tc_iot_hal_snprintf(current, buffer_left, HTTP_HEADER_FMT, header,
                              (int)strlen(val), val);
    if (ret > 0) {
        tc_iot_yabuffer_forward(&(request->buf), ret);
    }

    return ret;
}

int tc_iot_http_request_n_append_header(tc_iot_http_request* request,
                                        const char* header, const char* val,
                                        int val_len) {
    char* current = NULL;
    int buffer_left = 0;
    int ret = 0;
    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(header, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(val, TC_IOT_NULL_POINTER);

    current = tc_iot_yabuffer_current(&(request->buf));
    buffer_left = tc_iot_yabuffer_left(&(request->buf));
    ret = tc_iot_hal_snprintf(current, buffer_left, HTTP_HEADER_FMT, header,
                              val_len, val);
    if (ret > 0) {
        tc_iot_yabuffer_forward(&(request->buf), ret);
    }

    return ret;
}

int tc_iot_http_request_append_body(tc_iot_http_request* request,
                                    const char* body) {
    char* current = tc_iot_yabuffer_current(&(request->buf));
    int buffer_left = tc_iot_yabuffer_left(&(request->buf));

    int ret;

    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    if (body) {
        ret = tc_iot_hal_snprintf(current, buffer_left, HTTP_BODY_FMT, body);
    } else {
        ret = tc_iot_hal_snprintf(current, buffer_left, HTTP_BODY_FMT, "");
    }

    if (ret > 0) {
        tc_iot_yabuffer_forward(&(request->buf), ret);
    }

    return ret;
}

int tc_iot_create_http_request(tc_iot_http_request* request, const char* host,
                               int host_len, const char* method,
                               const char* abs_path, int abs_path_len,
                               const char* http_version, const char* user_agent,
                               const char* content_type, const char* body) {
    int body_len;
    char body_len_str[20];

    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(host, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(abs_path, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(http_version, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(user_agent, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(content_type, TC_IOT_NULL_POINTER);

    tc_iot_http_request_init(request, method, abs_path, abs_path_len,
                             http_version);
    tc_iot_http_request_append_header(request, HTTP_HEADER_USER_AGENT,
                                      user_agent);
    tc_iot_http_request_n_append_header(request, HTTP_HEADER_HOST, host,
                                        host_len);
    tc_iot_http_request_append_header(request, HTTP_HEADER_ACCEPT, "*/*");
    if (content_type) {
        tc_iot_http_request_append_header(request, HTTP_HEADER_CONTENT_TYPE,
                                          content_type);
    }
    tc_iot_http_request_append_header(
        request, HTTP_HEADER_ACCEPT_ENCODING,
        "identity"); /* accept orignal content only, no zip */

    if (body) {
        body_len = strlen(body);
        if (body_len) {
            tc_iot_hal_snprintf(body_len_str, sizeof(body_len_str), "%d",
                                body_len);
            tc_iot_http_request_append_header(
                request, HTTP_HEADER_CONTENT_LENGTH, body_len_str);
        }
    }
    tc_iot_http_request_append_body(request, body);
    return TC_IOT_SUCCESS;
}

int tc_iot_create_post_request(tc_iot_http_request* request,
                               const char* abs_path, int abs_path_len,
                               const char* host, int host_len,
                               const char* body) {
    return tc_iot_create_http_request(request, host, host_len, HTTP_POST, abs_path,
                               abs_path_len, HTTP_VER_1_0, "tciotclient/1.0",
                               HTTP_CONTENT_FORM_URLENCODED, body);
}

int tc_iot_calc_auth_sign(char* sign_out, int max_sign_len, const char* secret,
                          int secret_len, const char* client_id,
                          int client_id_len, const char* device_name,
                          int device_name_len, long expire, long nonce,
                          const char* product_id, int product_id_len,
                          long timestamp) {
    char buf[512];
    int buf_len = sizeof(buf);
    char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];
    int ret;
    char b64_buf[128];
    int data_len;
    int url_ret;

    IF_NULL_RETURN(sign_out, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(client_id, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);
    IF_EQUAL_RETURN(max_sign_len, 0, TC_IOT_INVALID_PARAMETER);

    memset(buf, 0, buf_len);
    if (client_id_len == strlen(client_id)
            && device_name_len == strlen(device_name)
            && product_id_len == strlen(product_id)) {
        data_len = tc_iot_hal_snprintf(
            buf, buf_len,
            "clientId=%s&deviceName=%s&expire=%ld&nonce=%ld&productId=%s&"
            "timestamp=%ld",
            client_id, device_name, expire, nonce,
            product_id, timestamp);
    } else {
        data_len = tc_iot_hal_snprintf(
                buf, buf_len,
                "deviceName=%.*s&nonce=%ld&productId=%.*s&timestamp=%ld",
                device_name_len, device_name, nonce,
                product_id_len, product_id, timestamp);
    }

    if (data_len >= buf_len) {
        TC_IOT_LOG_ERROR("generate_auth_sign buffer overflow.");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    tc_iot_hmac_sha256((unsigned char *)buf, data_len, (const unsigned char *)secret, secret_len, (unsigned char *)sha256_digest);

    ret = tc_iot_base64_encode((unsigned char *)sha256_digest, sizeof(sha256_digest), b64_buf,
                               sizeof(b64_buf));
    if (ret < sizeof(b64_buf) && ret > 0) {
       b64_buf[ret] = '\0'; 
    }

    TC_IOT_LOG_TRACE("tc_iot_calc_auth_sign source %s sec %s sig %s\n", buf, secret, b64_buf);
    url_ret = tc_iot_url_encode(b64_buf, ret, sign_out, max_sign_len);
    if (url_ret < max_sign_len) {
        sign_out[url_ret] = '\0';
    }

    return url_ret;
}

static int add_tc_iot_url_encoded_field(tc_iot_yabuffer_t* buffer,
                                        const char* prefix, const char* val,
                                        int val_len) {
    int total = 0;
    int ret = 0;

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(prefix, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(val, TC_IOT_NULL_POINTER);

    total = tc_iot_yabuffer_append(buffer, prefix);
    ret = tc_iot_url_encode(val, val_len, tc_iot_yabuffer_current(buffer),
                            tc_iot_yabuffer_left(buffer));

    tc_iot_yabuffer_forward(buffer, ret);
    total += ret;
    return total;
}

static int add_url_long_field(tc_iot_yabuffer_t* buffer, const char* prefix,
                              long val) {
    int total = 0;
    int ret;
    char* current;
    int buffer_left;

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(prefix, TC_IOT_NULL_POINTER);

    total = tc_iot_yabuffer_append(buffer, prefix);
    current = tc_iot_yabuffer_current(buffer);
    buffer_left = tc_iot_yabuffer_left(buffer);

    ret = tc_iot_hal_snprintf(current, buffer_left, "%ld", val);

    if (ret > 0) {
        tc_iot_yabuffer_forward(buffer, ret);
        total += ret;
        return total;
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }
}

int tc_iot_create_auth_request_form(char* form, int max_form_len,
                                    const char* secret, int secret_len,
                                    const char* client_id, int client_id_len,
                                    const char* device_name,
                                    int device_name_len, long expire,
                                    long nonce, const char* product_id,
                                    int product_id_len, long timestamp) {
    tc_iot_yabuffer_t form_buf;
    int total = 0;

    IF_NULL_RETURN(form, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(client_id, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);

    tc_iot_yabuffer_init(&form_buf, form, max_form_len);
    total += add_tc_iot_url_encoded_field(&form_buf, "clientId=", client_id,
                                          client_id_len);
    total += add_tc_iot_url_encoded_field(&form_buf, "&deviceName=",
                                          device_name, device_name_len);
    total += add_url_long_field(&form_buf, "&expire=", expire);
    total += add_url_long_field(&form_buf, "&nonce=", nonce);
    total += add_tc_iot_url_encoded_field(&form_buf, "&productId=", product_id,
                                          product_id_len);
    total += add_url_long_field(&form_buf, "&timestamp=", timestamp);
    total += add_tc_iot_url_encoded_field(&form_buf, "&signature=", "", 0);

    total += tc_iot_calc_auth_sign(
        tc_iot_yabuffer_current(&form_buf), tc_iot_yabuffer_left(&form_buf),
        secret, secret_len, client_id, client_id_len, device_name,
        device_name_len, expire, nonce, product_id, product_id_len, timestamp);
    return total;
}

static int tc_iot_calc_active_device_sign(char* sign_out, int max_sign_len, 
                            const char* product_secret, int secret_len,
                            const char* device_name, int device_name_len, 
                            const char* product_id,int product_id_len,
                            long nonce, 
                            long timestamp    ) {
    char buf[512];
    int buf_len = sizeof(buf);
    char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];
    int ret;
    char b64_buf[128];
    int data_len;
    int url_ret;

    IF_NULL_RETURN(sign_out, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);
    IF_EQUAL_RETURN(max_sign_len, 0, TC_IOT_INVALID_PARAMETER);

    data_len = tc_iot_hal_snprintf(
        buf, buf_len,
        "deviceName=%.*s&nonce=%ld&productId=%.*s&timestamp=%ld",
        device_name_len, device_name, nonce,
        product_id_len, product_id, timestamp);
    
    
    if (data_len >= buf_len) {
        TC_IOT_LOG_ERROR("generate_active_device_sign buffer overflow.");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    tc_iot_hmac_sha256((unsigned char *)buf, data_len, (const unsigned char *)product_secret, secret_len, (unsigned char *)sha256_digest);
    
    
    ret = tc_iot_base64_encode((unsigned char *)sha256_digest, sizeof(sha256_digest), b64_buf,
                               sizeof(b64_buf));
    url_ret = tc_iot_url_encode(b64_buf, ret, sign_out, max_sign_len);
    
    /* TC_IOT_LOG_DEBUG(" tc_iot_url_encoded sign\n %.*s\n, url_ret=%d", url_ret, sign_out, url_ret);  */
    if (url_ret < max_sign_len) {
        sign_out[url_ret] = '\0';
    }
    TC_IOT_LOG_DEBUG(" tc_iot_calc_active_device_sign  source:%s sign:%s", buf , sign_out);
    return url_ret;
}

int tc_iot_create_active_device_form(char* form, int max_form_len,
                                    const char* product_secret, int secret_len,
                                    const char* device_name, int device_name_len, 
                                    const char* product_id,int product_id_len,
                                    long nonce, long timestamp) {
    tc_iot_yabuffer_t form_buf;
    int total = 0;
    
    IF_NULL_RETURN(form, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_secret, TC_IOT_NULL_POINTER);
    
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);

    tc_iot_yabuffer_init(&form_buf, form, max_form_len);
    
    total += add_tc_iot_url_encoded_field(&form_buf, "productId=", product_id,
                                      product_id_len);
    total += add_tc_iot_url_encoded_field(&form_buf, "&deviceName=",
                                          device_name, device_name_len);
    
    total += add_url_long_field(&form_buf, "&nonce=", nonce);

    total += add_url_long_field(&form_buf, "&timestamp=", timestamp);
    total += add_tc_iot_url_encoded_field(&form_buf, "&signature=", "", 0);

    total += tc_iot_calc_active_device_sign(
        tc_iot_yabuffer_current(&form_buf), tc_iot_yabuffer_left(&form_buf),
        product_secret, secret_len, 
        device_name, device_name_len, product_id, product_id_len,
        nonce, timestamp);
    return total;
}

int tc_iot_parse_http_response_code(const char * resp) {
    int ret;
    int i;

    IF_NULL_RETURN(resp, TC_IOT_NULL_POINTER);
    if (strncmp(HTTP_RESPONSE_STATE_PREFIX, resp, HTTP_RESPONSE_STATE_PREFIX_LEN) != 0) {
        return TC_IOT_HTTP_RESPONSE_INVALID;
    }

    resp+= HTTP_RESPONSE_STATE_PREFIX_LEN;
    if (*resp != '0' && *resp != '1') {
        TC_IOT_LOG_TRACE("http minor version invalid: %s", tc_iot_log_summary_string(resp, 5));
        return TC_IOT_HTTP_RESPONSE_INVALID;
    }
    resp++;
    if (*resp != ' ') {
        TC_IOT_LOG_TRACE("http stat line invalid: %s", tc_iot_log_summary_string(resp, 5));
        return TC_IOT_HTTP_RESPONSE_INVALID;
    }
    resp++;
    ret = 0;
    for ( i = 0; i < 3; i++, resp++) {
        if (*resp < '0') {
            return TC_IOT_HTTP_RESPONSE_INVALID;
        }
        if (*resp > '9') {
            return TC_IOT_HTTP_RESPONSE_INVALID;
        }
        ret = ret*10 +((*resp) - '0');
    }

    return ret;
}

#ifdef __cplusplus
}
#endif
