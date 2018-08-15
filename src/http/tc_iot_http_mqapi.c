#include "tc_iot_inc.h"

int tc_iot_http_mqapi_rpc( char * result, int result_len,
        const char* api_url, char* root_ca_path, long timestamp, long nonce,
        tc_iot_device_info* p_device_info, const char * message) {

    char sign_out[TC_IOT_HTTP_MQAPI_REQUEST_FORM_LEN];
    char http_buffer[TC_IOT_HTTP_MQAPI_RESPONSE_LEN];
    tc_iot_network_t network;
    tc_iot_http_request request;
    int ret;
    char* rsp_body;
    tc_iot_net_context_init_t netcontext;
#ifdef ENABLE_TLS
    tc_iot_tls_config_t* config;
#endif

    jsmn_parser p;
    jsmntok_t t[20];

    char temp_buf[TC_IOT_HTTP_MAX_URL_LENGTH];
    int json_field_index = 0;
    int r;
    int i;
    int temp_len;
    int redirect_count = 0;

    memset(&netcontext, 0, sizeof(netcontext));

    IF_NULL_RETURN(api_url, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_device_info, TC_IOT_NULL_POINTER);

    ret = tc_iot_create_mqapi_rpc_json( sign_out, sizeof(sign_out), 
                                        p_device_info->secret,
                                        p_device_info->device_name,
                                        message,
                                        nonce,
                                        p_device_info->product_id,
                                        timestamp);
    
    tc_iot_mem_usage_log("sign_out[TC_IOT_HTTP_MQAPI_REQUEST_FORM_LEN]", sizeof(sign_out), ret);

    TC_IOT_LOG_TRACE("signed request form:\n%s", sign_out);

    memset(&network, 0, sizeof(network));

parse_url:

    TC_IOT_LOG_TRACE("request url=%s", api_url);
    if (strncmp(api_url, HTTPS_PREFIX, HTTPS_PREFIX_LEN) == 0) {
#ifdef ENABLE_TLS
        netcontext.fd = -1;
        netcontext.use_tls = 1;

        config = &(netcontext.tls_config);
        config->root_ca_in_mem = g_tc_iot_https_root_ca_certs;
        if (root_ca_path) {
            config->root_ca_location = root_ca_path;
        }
        config->timeout_ms = TC_IOT_DEFAULT_TLS_HANSHAKE_TIMEOUT_MS;
        if (netcontext.use_tls) {
            config->verify_server = TC_IOT_HTTPS_CERT_STRICT_CHECK;
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

    tc_iot_yabuffer_init(&request.buf, (char *)http_buffer,
                         sizeof(http_buffer));
    /* request init end */

    memset(http_buffer, 0, sizeof(http_buffer));
    TC_IOT_LOG_TRACE("request url=%s", api_url);
    ret = http_post_json(&network, &request, api_url, sign_out, http_buffer,
                               sizeof(http_buffer), 2000);

    tc_iot_mem_usage_log("http_buffer[TC_IOT_HTTP_MQAPI_RESPONSE_LEN]", sizeof(http_buffer), strlen(http_buffer));

    ret = tc_iot_parse_http_response_code(http_buffer);
    if (ret != 200) {
        if (ret == 301 || ret == 302) {

            if (redirect_count < 5) {
                redirect_count++;
            } else {
                TC_IOT_LOG_ERROR("http code %d, redirect exceed maxcount=%d.", ret, redirect_count);
                return TC_IOT_HTTP_REDIRECT_TOO_MANY;
            }

            rsp_body = strstr(http_buffer, "Location: ");
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
                TC_IOT_LOG_TRACE("http response status code=%d, redirect times=%d, new_url=%s", ret, redirect_count, api_url);
            } else {
                TC_IOT_LOG_ERROR("http code %d, Location header not found.", ret);
            }

            goto parse_url;
        } else {
            TC_IOT_LOG_WARN("http response status code=%d", ret);
        }
        return TC_IOT_HTTP_RPC_FAILED;
    }

    rsp_body = strstr(http_buffer, "\r\n\r\n");
    if (rsp_body) {
        /* skip \r\n\r\n */
        jsmn_init(&p);
        rsp_body += 4;
        r = jsmn_parse(&p, rsp_body, strlen(rsp_body), t,
                       sizeof(t) / sizeof(t[0]));
        if (r < 0) {
            TC_IOT_LOG_ERROR("Failed to parse JSON: %s", rsp_body);
            return TC_IOT_JSON_PARSE_FAILED;
        }

        if (r < 1 || t[0].type != JSMN_OBJECT) {
            TC_IOT_LOG_ERROR("Invalid JSON: %s", rsp_body);
            return TC_IOT_JSON_PARSE_FAILED;
        }

        json_field_index = tc_iot_json_find_token(rsp_body, t, r, "returnCode",
                                                 temp_buf, sizeof(temp_buf));
        if (json_field_index <= 0 || strlen(temp_buf) != 1 ||
            temp_buf[0] != '0') {
            TC_IOT_LOG_ERROR("failed to fetch token %d/%s: %s", json_field_index,
                      temp_buf, rsp_body);
            return TC_IOT_HTTP_RPC_FAILED;
        }

        json_field_index = tc_iot_json_find_token(rsp_body, t, r, "data.message",
                                                 result, result_len);
        if (json_field_index <= 0) {
            TC_IOT_LOG_ERROR("failed to fetch token %d/%s: %s", json_field_index,
                      temp_buf, rsp_body);
            return TC_IOT_HTTP_RPC_FAILED;
        }

        return strlen(result);
    } else {
        return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
    }
}

