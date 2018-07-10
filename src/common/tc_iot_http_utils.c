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
                               const char* body, const char * content_type) {
    return tc_iot_create_http_request(request, host, host_len, HTTP_POST, abs_path,
                               abs_path_len, HTTP_VER_1_0,TC_IOT_USER_AGENT,
                               content_type, body);
}

int tc_iot_create_get_request(tc_iot_http_request* request,
                               const char* abs_path, int abs_path_len,
                               const char* host, int host_len) {
    return tc_iot_create_http_request(request, host, host_len, HTTP_GET, abs_path,
                               abs_path_len, HTTP_VER_1_0,TC_IOT_USER_AGENT,
                               NULL, NULL);
}

int tc_iot_create_head_request(tc_iot_http_request* request,
                               const char* abs_path, int abs_path_len,
                               const char* host, int host_len) {
    return tc_iot_create_http_request(request, host, host_len, HTTP_HEAD, abs_path,
                               abs_path_len, HTTP_VER_1_0,TC_IOT_USER_AGENT,
                               NULL, NULL);
}

int tc_iot_calc_auth_sign(char* sign_out, int max_sign_len, const char* secret, const char* client_id, const char* device_name,
                          long expire, long nonce,
                          const char* product_id,
                          long timestamp) {
    unsigned char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];
    int ret;
    char b64_buf[TC_IOT_BASE64_ENCODE_OUT_LEN(TC_IOT_SHA256_DIGEST_SIZE)];
    int url_ret;

    IF_NULL_RETURN(sign_out, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(client_id, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);
    IF_EQUAL_RETURN(max_sign_len, 0, TC_IOT_INVALID_PARAMETER);

    ret = tc_iot_calc_sign(
        sha256_digest, sizeof(sha256_digest),
        secret,
        "clientId=%s&deviceName=%s&expire=%ld&nonce=%ld&productId=%s&timestamp=%ld",
        client_id, device_name, expire, nonce,
        product_id, timestamp);

    ret = tc_iot_base64_encode((unsigned char *)sha256_digest, sizeof(sha256_digest), b64_buf,
                               sizeof(b64_buf));
    if (ret < sizeof(b64_buf) && ret > 0) {
       b64_buf[ret] = '\0'; 
       tc_iot_mem_usage_log("b64_buf", sizeof(b64_buf), ret);
    }

    TC_IOT_LOG_TRACE("tc_iot_calc_auth_sign source clientId=%s&deviceName=%s&expire=%ld&nonce=%ld&productId=%s&timestamp=%ld sec %s sig %s\n", 
            client_id, device_name, expire, nonce,product_id, timestamp,
            secret, b64_buf);

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
                                    const char* secret,
                                    const char* client_id,
                                    const char* device_name,
                                    long expire,
                                    long nonce,
                                    const char* product_id,
                                    long timestamp) {
    tc_iot_yabuffer_t form_buf;
    int total = 0;

    IF_NULL_RETURN(form, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(client_id, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);

    tc_iot_yabuffer_init(&form_buf, form, max_form_len);
    total += add_tc_iot_url_encoded_field(&form_buf, "clientId=", client_id,
                                          strlen(client_id));
    total += add_tc_iot_url_encoded_field(&form_buf, "&deviceName=",
                                          device_name, strlen(device_name));
    total += add_url_long_field(&form_buf, "&expire=", expire);
    total += add_url_long_field(&form_buf, "&nonce=", nonce);
    total += add_tc_iot_url_encoded_field(&form_buf, "&productId=", product_id,
                                          strlen(product_id));
    total += add_url_long_field(&form_buf, "&timestamp=", timestamp);
    total += add_tc_iot_url_encoded_field(&form_buf, "&signature=", "", 0);

    total += tc_iot_calc_auth_sign(
        tc_iot_yabuffer_current(&form_buf), tc_iot_yabuffer_left(&form_buf),
        secret, client_id, device_name,
        expire, nonce, product_id, timestamp);
    return total;
}

static int tc_iot_calc_active_device_sign(char* sign_out, int max_sign_len, 
                            const char* product_secret,
                            const char* device_name,  
                            const char* product_id,
                            long nonce, 
                            long timestamp    ) {
    unsigned char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];
    int ret;
    char b64_buf[TC_IOT_BASE64_ENCODE_OUT_LEN(TC_IOT_SHA256_DIGEST_SIZE)];
    int url_ret;

    IF_NULL_RETURN(sign_out, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);
    IF_EQUAL_RETURN(max_sign_len, 0, TC_IOT_INVALID_PARAMETER);

    ret = tc_iot_calc_sign(
        sha256_digest, sizeof(sha256_digest),
        product_secret,
        "deviceName=%s&nonce=%ld&productId=%s&timestamp=%ld",
        device_name, nonce,
        product_id, timestamp);

    ret = tc_iot_base64_encode((unsigned char *)sha256_digest, sizeof(sha256_digest), b64_buf,
                               sizeof(b64_buf));

    tc_iot_mem_usage_log("b64_buf", sizeof(b64_buf), ret);

    url_ret = tc_iot_url_encode(b64_buf, ret, sign_out, max_sign_len);
    
    /* TC_IOT_LOG_DEBUG(" tc_iot_url_encoded sign\n %.*s\n, url_ret=%d", url_ret, sign_out, url_ret);  */
    if (url_ret < max_sign_len) {
        sign_out[url_ret] = '\0';
    }
    TC_IOT_LOG_DEBUG(" tc_iot_calc_active_device_sign deviceName=%s&nonce=%ld&productId=%s&timestamp=%ld sign:%s", 
            device_name, nonce,product_id, timestamp , sign_out);
    return url_ret;
}

int tc_iot_create_active_device_form(char* form, int max_form_len,
                                    const char* product_secret, 
                                    const char* device_name,  
                                    const char* product_id,
                                    long nonce, long timestamp) {
    tc_iot_yabuffer_t form_buf;
    int total = 0;
    
    IF_NULL_RETURN(form, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_secret, TC_IOT_NULL_POINTER);
    
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);

    tc_iot_yabuffer_init(&form_buf, form, max_form_len);
    
    total += add_tc_iot_url_encoded_field(&form_buf, "productId=", product_id,
                                      strlen(product_id));
    total += add_tc_iot_url_encoded_field(&form_buf, "&deviceName=",
                                          device_name, strlen(device_name));
    
    total += add_url_long_field(&form_buf, "&nonce=", nonce);

    total += add_url_long_field(&form_buf, "&timestamp=", timestamp);
    total += add_tc_iot_url_encoded_field(&form_buf, "&signature=", "", 0);

    total += tc_iot_calc_active_device_sign(
        tc_iot_yabuffer_current(&form_buf), tc_iot_yabuffer_left(&form_buf),
        product_secret,  
        device_name, product_id, 
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

int tc_iot_http_get(tc_iot_network_t* network,
                         tc_iot_http_request* request,
                         const char* url, 
                         int timeout_ms, const char * extra_header) {
    tc_iot_url_parse_result_t result;
    char temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH];
    int written_len;
    int ret = tc_iot_url_parse(url, strlen(url), &result);

    if (ret < 0) {
        return ret;
    }

    if (result.host_len >= sizeof(temp_host)) {
        TC_IOT_LOG_ERROR("host address too long.");
        return -1;
    }

    if (result.over_tls != network->net_context.use_tls) {
        TC_IOT_LOG_WARN("network type not match: url tls=%d, context tls=%d",
                 (int)result.over_tls, (int)network->net_context.use_tls);
        return -1;
    }

    strncpy(temp_host, url + result.host_start, result.host_len);
    temp_host[result.host_len] = '\0';
    tc_iot_mem_usage_log("temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH]", sizeof(temp_host), result.host_len);

    TC_IOT_LOG_TRACE("remote=%s:%d", temp_host, result.port);

    network->do_connect(network, temp_host, result.port);

    if (result.path_len) {
        ret = tc_iot_create_get_request(
            request, url + result.path_start, result.path_len,
            url + result.host_start, result.host_len);
    } else {
        ret = tc_iot_create_get_request(request, "/", 1, url + result.host_start,
                                       result.host_len);
    }

    if (extra_header != NULL && extra_header[0] != '\0') {
        tc_iot_yabuffer_forward(&request->buf, -2);
        /* Range: bytes=%d- */
        ret = tc_iot_hal_snprintf(tc_iot_yabuffer_current(&request->buf), 
                tc_iot_yabuffer_left(&request->buf),
                "%s\r\n\r\n", extra_header
                );
        tc_iot_yabuffer_forward(&request->buf, ret);
        if (tc_iot_yabuffer_left(&request->buf) <= 0) {
            TC_IOT_LOG_ERROR("request buffer size=%d not enough", 
                    tc_iot_yabuffer_len(&request->buf));
            return TC_IOT_BUFFER_OVERFLOW;
        }
    }
    written_len = network->do_write(network, (unsigned char *)request->buf.data,
                                    request->buf.pos, timeout_ms);
    TC_IOT_LOG_TRACE("request with:\n%s", request->buf.data);
    if (written_len == request->buf.pos) {
        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_FAILURE;
    }
}

int tc_iot_http_head(tc_iot_network_t* network,
                         tc_iot_http_request* request, const char* url,
                         int timeout_ms) {
    tc_iot_url_parse_result_t result;
    char temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH];
    int written_len;
    int ret = tc_iot_url_parse(url, strlen(url), &result);

    if (ret < 0) {
        return ret;
    }

    if (result.path_len) {
        ret = tc_iot_create_head_request(
            request, url + result.path_start, result.path_len,
            url + result.host_start, result.host_len);
    } else {
        ret =
            tc_iot_create_head_request(request, "/", 1, url + result.host_start,
                                       result.host_len);
    }

    if (result.host_len >= sizeof(temp_host)) {
        TC_IOT_LOG_ERROR("host address too long.");
        return -1;
    }

    tc_iot_mem_usage_log("temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH]", sizeof(temp_host), result.host_len);

    if (result.over_tls != network->net_context.use_tls) {
        TC_IOT_LOG_WARN("network type not match: url tls=%d, context tls=%d",
                 (int)result.over_tls, (int)network->net_context.use_tls);
        return -1;
    }

    strncpy(temp_host, url + result.host_start, result.host_len);
    temp_host[result.host_len] = '\0';

    TC_IOT_LOG_TRACE("remote=%s:%d", temp_host, result.port);

    network->do_connect(network, temp_host, result.port);
    written_len = network->do_write(network, (unsigned char *)request->buf.data,
                                    request->buf.pos, timeout_ms);
    TC_IOT_LOG_TRACE("request with:\n%s", request->buf.data);
    if (written_len == request->buf.pos) {
        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_FAILURE;
    }
}

static int http_post_data(tc_iot_network_t* network,
                         tc_iot_http_request* request, const char* url,
                         const char* encoded_body, char* resp, int resp_max_len,
                         int timeout_ms, const char * content_type) {
    tc_iot_url_parse_result_t result;
    char temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH];
    int read_len;
    int written_len;
    int ret = tc_iot_url_parse(url, strlen(url), &result);

    if (ret < 0) {
        return ret;
    }

    if (result.path_len) {
        ret = tc_iot_create_post_request(
            request, url + result.path_start, result.path_len,
            url + result.host_start, result.host_len, encoded_body, content_type);
    } else {
        ret =
            tc_iot_create_post_request(request, "/", 1, url + result.host_start,
                                       result.host_len, encoded_body, content_type);
    }

    if (result.host_len >= sizeof(temp_host)) {
        TC_IOT_LOG_ERROR("host address too long.");
        return -1;
    }

    if (result.over_tls != network->net_context.use_tls) {
        TC_IOT_LOG_WARN("network type not match: url tls=%d, context tls=%d",
                 (int)result.over_tls, (int)network->net_context.use_tls);
        return -1;
    }

    strncpy(temp_host, url + result.host_start, result.host_len);
    temp_host[result.host_len] = '\0';
    tc_iot_mem_usage_log("temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH]", sizeof(temp_host), result.host_len);

    TC_IOT_LOG_TRACE("remote=%s:%d", temp_host, result.port);

    network->do_connect(network, temp_host, result.port);
    written_len = network->do_write(network, (unsigned char *)request->buf.data,
                                    request->buf.pos, timeout_ms);

    TC_IOT_LOG_TRACE("request with,len=%d:\n%s", written_len, request->buf.data);
    read_len = network->do_read(network, (unsigned char *)resp, resp_max_len, timeout_ms);
    if (resp_max_len > read_len) {
        resp[read_len] = '\0';
    }
    TC_IOT_LOG_TRACE("response with:\n%s", resp);

    network->do_disconnect(network);

    return read_len;
}

int http_post_urlencoded(tc_iot_network_t* network,
                         tc_iot_http_request* request, const char* url,
                         const char* encoded_body, char* resp, int resp_max_len,
                         int timeout_ms) {
    return http_post_data(network, request, url, encoded_body, resp, resp_max_len, timeout_ms, HTTP_CONTENT_FORM_URLENCODED);
}

int http_post_json(tc_iot_network_t* network,
                         tc_iot_http_request* request, const char* url,
                         const char* json_body, char* resp, int resp_max_len,
                         int timeout_ms) {
    return http_post_data(network, request, url, json_body, resp, resp_max_len, timeout_ms, HTTP_CONTENT_JSON);
}

int tc_iot_calc_sign(unsigned char * output, int output_len, const char * secret, const char * format, ...) {
    va_list ap;
    const unsigned char * pos  = (const unsigned char *)format;
    const unsigned char * prev = (const unsigned char *)format;
    char num_buf[20];
    char format_str[4];
    const char * var_str;

    tc_iot_hmac_sha256_t hmac;
    tc_iot_hmac_sha256_init(&hmac, (unsigned char *)secret, strlen(secret));
    TC_IOT_LOG_TRACE("secret=%s,format=%s",secret, format);

    if (output_len < TC_IOT_SHA256_DIGEST_SIZE) {
        return TC_IOT_BUFFER_OVERFLOW;
    }

    va_start(ap, format);
    
    while(*pos) {
        if ('%' == *pos) {
            pos++;
            if ((pos-1) > prev) {
                tc_iot_sha256_update(&(hmac.sha), prev, pos-1-prev);
                prev = pos;
            }
            switch(*pos) {
                case 'c':
                case 'd':
                case 'i':
                    tc_iot_hal_snprintf(format_str, sizeof(format_str), "%%%c", *pos);
                    tc_iot_hal_snprintf(num_buf, sizeof(num_buf), format_str, va_arg(ap, int));
                    tc_iot_sha256_update(&(hmac.sha), (const unsigned char *)num_buf, strlen(num_buf));
                    break;
                case 'u':
                case 'o':
                case 'x':
                case 'X':
                    tc_iot_hal_snprintf(format_str, sizeof(format_str), "%%%c", *pos);
                    tc_iot_hal_snprintf(num_buf, sizeof(num_buf), format_str, va_arg(ap, unsigned int));
                    tc_iot_sha256_update(&(hmac.sha), (const unsigned char *)num_buf, strlen(num_buf));
                    break;
                case 'f':
                case 'F':
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                case 'a':
                    tc_iot_hal_snprintf(format_str, sizeof(format_str), "%%%c", *pos);
                    tc_iot_hal_snprintf(num_buf, sizeof(num_buf), format_str, va_arg(ap, double));
                    tc_iot_sha256_update(&(hmac.sha), (const unsigned char *)num_buf, strlen(num_buf));
                    break;
                case 's':
                    var_str = va_arg(ap, const char *);
                    tc_iot_sha256_update(&(hmac.sha), (const unsigned char *)var_str, strlen(var_str));
                    break;

                case 'l':
                    pos++;
                    switch(*pos) {
                        case 'd':
                        case 'i':
                            tc_iot_hal_snprintf(format_str, sizeof(format_str), "%%l%c", *pos);
                            tc_iot_hal_snprintf(num_buf, sizeof(num_buf), format_str, va_arg(ap, long int));
                            tc_iot_sha256_update(&(hmac.sha), (const unsigned char *)num_buf, strlen(num_buf));
                            break;
                        case 'u':
                        case 'o':
                        case 'x':
                        case 'X':
                            tc_iot_hal_snprintf(format_str, sizeof(format_str), "%%l%c", *pos);
                            tc_iot_hal_snprintf(num_buf, sizeof(num_buf), format_str, va_arg(ap, unsigned long int));
                            tc_iot_sha256_update(&(hmac.sha), (const unsigned char *)num_buf, strlen(num_buf));
                            break;
                        default:
                            TC_IOT_LOG_ERROR("unkown *pos=%c", *pos);
                            va_end(ap);
                            return TC_IOT_INVALID_PARAMETER;
                    }
                    break;
                case '%':
                    pos++;
                    continue;
                default:
                    TC_IOT_LOG_ERROR("unkown *pos=%c", *pos);
                    va_end(ap);
                    return TC_IOT_INVALID_PARAMETER;
            }
            pos++;
            prev = pos;
        } else {
            pos++;
        }
    }

    if ((pos-1) > prev) {
        tc_iot_sha256_update(&(hmac.sha), prev, pos-1-prev);
    }

    tc_iot_hmac_sha256_finish(&hmac, NULL, 0);

    memcpy(output, hmac.digest, TC_IOT_SHA256_DIGEST_SIZE);

    va_end(ap);

    return TC_IOT_SHA256_DIGEST_SIZE;    
}

int tc_iot_create_mqapi_rpc_json(char* form, int max_form_len,
                                    const char* secret,
                                    const char* device_name,
                                    const char* message,
                                    long nonce,
                                    const char* product_id,
                                    long timestamp
                                    ) {
    unsigned char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];
    int ret;
    char b64_buf[TC_IOT_BASE64_ENCODE_OUT_LEN(TC_IOT_SHA256_DIGEST_SIZE)];
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    IF_NULL_RETURN(form, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);

    ret = tc_iot_calc_sign(
            sha256_digest, sizeof(sha256_digest),
            secret,
            "deviceName=%s&message=%s&nonce=%ld&productId=%s&timestamp=%ld",
             device_name,message,nonce,product_id,timestamp
        );

    ret = tc_iot_base64_encode((unsigned char *)sha256_digest, sizeof(sha256_digest), b64_buf,
                               sizeof(b64_buf));
    if (ret < sizeof(b64_buf) && ret > 0) {
       b64_buf[ret] = '\0'; 
       tc_iot_mem_usage_log("b64_buf", sizeof(b64_buf), ret);
    } else {
        TC_IOT_LOG_ERROR("b64_buf for sha256 digest overflow, ret=%d.", ret);
        if (ret < 0) {
            return ret;
        } else {
            return TC_IOT_BUFFER_OVERFLOW;
        }
    }

    tc_iot_json_writer_open(w, form, max_form_len);
    tc_iot_json_writer_string(w ,"productId", product_id);
    tc_iot_json_writer_string(w ,"deviceName", device_name);
    tc_iot_json_writer_int(w ,"nonce", nonce);
    tc_iot_json_writer_int(w ,"timestamp", timestamp);
    tc_iot_json_writer_string(w ,"message", message);
    tc_iot_json_writer_string(w ,"signature", b64_buf);
    ret = tc_iot_json_writer_close(w);

    return ret;
}

