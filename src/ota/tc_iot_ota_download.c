#include "tc_iot_inc.h"

int tc_iot_ota_download(const char* api_url, int partial_start, tc_iot_http_download_callback download_callback, const void * context) {
    tc_iot_network_t network;
    tc_iot_http_request request;
    unsigned char http_request_buffer[1024];
    char http_resp[1024];
    int max_http_resp_len = sizeof(http_resp) - 1;
    char temp_buf[128];
    int ret;
    char* rsp_body;
    int redirect_count = 0;
    int temp_len = 0;
    int i = 0;
    int http_code = 0;
    char * content_length_pos = NULL;
    int content_length = 0;
    int received_bytes = 0;

    tc_iot_net_context_init_t netcontext;
#ifdef ENABLE_TLS
    tc_iot_tls_config_t* config;
#endif

    memset(&netcontext, 0, sizeof(netcontext));
    IF_NULL_RETURN(api_url, TC_IOT_NULL_POINTER);
    memset(&network, 0, sizeof(network));

parse_url:

    TC_IOT_LOG_TRACE("request url=%s", api_url);
    if (strncmp(api_url, HTTPS_PREFIX, HTTPS_PREFIX_LEN) == 0) {
#ifdef ENABLE_TLS
        netcontext.fd = -1;
        netcontext.use_tls = 1;

        config = &(netcontext.tls_config);
        config->root_ca_in_mem = g_tc_iot_https_root_ca_certs;
        config->timeout_ms = 10000;
        if (netcontext.use_tls) {
            config->verify_server = 1;
        }

        tc_iot_hal_tls_init(&network, &netcontext);
        /* init network end*/
        TC_IOT_LOG_TRACE("tls network intialized.");
#else
        TC_IOT_LOG_FATAL("tls network not supported.");
        return TC_IOT_TLS_NOT_SUPPORTED;
#endif
    } else {
        memset(&netcontext, 0, sizeof(netcontext));
        tc_iot_hal_net_init(&network, &netcontext);
        TC_IOT_LOG_TRACE("dirtect tcp network intialized.");
    }

    /* request init begin */

    tc_iot_yabuffer_init(&request.buf, (char *)http_request_buffer,
                         sizeof(http_request_buffer));
    /* request init end */

    http_resp[sizeof(http_resp)-1] = 0;
    TC_IOT_LOG_TRACE("request url=%s", api_url);
    ret = tc_iot_http_get(&network, &request, api_url, http_resp, max_http_resp_len, 2000, partial_start);
    if (ret >= max_http_resp_len) {
        http_code = tc_iot_parse_http_response_code(http_resp);
        if (http_code != 200 && http_code != 206) {
            TC_IOT_LOG_ERROR("http resoponse http_code = %d", http_code);
            return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
        }
        content_length_pos = strstr(http_resp, HTTP_HEADER_CONTENT_LENGTH);
        if (content_length_pos == NULL) {
            TC_IOT_LOG_ERROR("%s header not found in large response, stop parseing", HTTP_HEADER_CONTENT_LENGTH);
            return TC_IOT_HTTP_RESPONSE_NO_CONTENT_LENGTH;
        }
        /* Content-Length: 70105
         * Skip Content-Length and ": "
         * */
        content_length_pos += strlen(HTTP_HEADER_CONTENT_LENGTH) + 2;
        while((*content_length_pos) >= '0' && (*content_length_pos) <= '9') {
            content_length = content_length*10 + ((*content_length_pos) - '0');
            content_length_pos++;
        }
        TC_IOT_LOG_TRACE("%s = %d", HTTP_HEADER_CONTENT_LENGTH, content_length);

        rsp_body = strstr(http_resp, "\r\n\r\n");

        if (rsp_body) {
            *(rsp_body+2) = '\0';
            TC_IOT_LOG_TRACE("%s", http_resp);
            rsp_body += 4;
            received_bytes = http_resp + ret - rsp_body;
            if (download_callback) {
                download_callback(context, rsp_body, received_bytes, 0 , content_length);
            }
            while( ret >= 0) {
                ret = network.do_read(&network,http_resp, max_http_resp_len, 2000);
                if ((ret <= max_http_resp_len) && (ret > 0)) {
                    http_resp[ret] = '\0';
                    if (download_callback) {
                        download_callback(context, http_resp, ret, received_bytes , content_length);
                    }
                    received_bytes += ret;
                } else if (ret == 0){
                    TC_IOT_LOG_TRACE("server closed connection, ret = %d", ret);
                    break;
                } else {
                    TC_IOT_LOG_ERROR("http continue request error ret = %d", ret);
                    break;
                }
            }
            TC_IOT_LOG_TRACE("%s=%d, received_bytes=%d", HTTP_HEADER_CONTENT_LENGTH, content_length, received_bytes);
            return TC_IOT_SUCCESS;
        } else {
            TC_IOT_LOG_ERROR("http response body not found");
            return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
        }
    } else if (ret <= 0) {
        TC_IOT_LOG_ERROR("http request ret = %d", ret);
        return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
    } else {
        /* normally return, do nothing but continue processing*/
    }

    http_code = tc_iot_parse_http_response_code(http_resp);
    if (http_code != 200) {
        if (http_code == 301 || http_code == 302) {

            if (redirect_count < 5) {
                redirect_count++;
            } else {
                TC_IOT_LOG_ERROR("http code %d, redirect exceed maxcount=%d.", http_code, redirect_count);
                return TC_IOT_HTTP_REDIRECT_TOO_MANY;
            }

            rsp_body = strstr(http_resp, "Location: ");
            if (rsp_body) {
                rsp_body += strlen("Location: ");
                temp_len = strlen(rsp_body);

                for (i = 0; i < temp_len; i++) {
                    temp_buf[i] = rsp_body[i];
                    if (temp_buf[i] == '\r') {
                        TC_IOT_LOG_TRACE("truncate api url");
                        temp_buf[i] = '\0';
                    }
                    if (temp_buf[i] == '\0') {
                        break;
                    }
                }
                api_url = temp_buf;
                memset(&netcontext, 0, sizeof(netcontext));
                TC_IOT_LOG_TRACE("http response status code=%d, redirect times=%d, new_url=%s", 
                        ret, redirect_count, api_url);
            } else {
                TC_IOT_LOG_ERROR("http code %d, Location header not found.", ret);
            }

            goto parse_url;
        } else {
            TC_IOT_LOG_WARN("http response status code=%d\n%s", http_code, http_resp);
        }
        return TC_IOT_REFRESH_TOKEN_FAILED;
    }


    rsp_body = strstr(http_resp, "\r\n\r\n");
    if (rsp_body) {
        rsp_body += 4;
        TC_IOT_LOG_TRACE("\nbody:\n%s\n", rsp_body);
        if (download_callback) {
            download_callback(context, rsp_body, strlen(rsp_body), 0 , strlen(rsp_body));
        }
        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
    }
}
