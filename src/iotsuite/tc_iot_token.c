#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

#define HTTPS_PREFIX "https"
#define HTTPS_PREFIX_LEN (sizeof(HTTPS_PREFIX) - 1)

int http_post_urlencoded(tc_iot_network_t* network,
                         tc_iot_http_request* request, const char* url,
                         const char* encoded_body, char* resp, int resp_max_len,
                         int timeout_ms) {
    tc_iot_url_parse_result_t result;
    char temp_host[1024];
    int written_len;	
    int read_len;													 
    int ret = tc_iot_url_parse(url, strlen(url), &result);
    if (ret < 0) {
        return ret;
    }

    if (result.path_len) {
        ret = tc_iot_create_post_request(
            request, url + result.path_start, result.path_len,
            url + result.host_start, result.host_len, encoded_body);
    } else {
        ret =
            tc_iot_create_post_request(request, "/", 1, url + result.host_start,
                                       result.host_len, encoded_body);
    }

    
    if (result.host_len >= sizeof(temp_host)) {
        LOG_ERROR("host address too long.");
        return -1;
    }

    if (result.over_tls != network->net_context.use_tls) {
        LOG_WARN("network type not match: url tls=%d, context tls=%d",
                  (int)result.over_tls, (int)network->net_context.use_tls);
        return -1;
    }

    strncpy(temp_host, url + result.host_start, result.host_len);
    temp_host[result.host_len] = '\0';

    LOG_TRACE("remote=%s:%d", temp_host, result.port);

    network->do_connect(network, temp_host, result.port);
    written_len = network->do_write(network, request->buf.data,
                                        request->buf.pos, timeout_ms);
    LOG_TRACE("request with:\n%.*s", written_len, request->buf.data);
    read_len = network->do_read(network, resp, resp_max_len, timeout_ms);
    LOG_TRACE("response with:\n%.*s", read_len, resp);

    network->do_disconnect(network);

    return read_len;
}

int http_refresh_auth_token(const char* api_url, char* root_ca_path,
                            tc_iot_device_info* p_device_info	) {
    char sign_out[1024];
	char http_resp[2048];
    long timestamp = tc_iot_hal_timestamp(NULL);
    
    long nonce = tc_iot_hal_random();
    long expire = 60;
	int sign_len;
	tc_iot_network_t network;			
    tc_iot_http_request request;
    unsigned char http_request_buffer[2048];	
	int ret;		
	char* rsp_body;		
	tc_iot_net_context_t netcontext;

	memset(&netcontext, 0, sizeof(netcontext));
	//netcontext.eth = eth;
	
	tc_iot_hal_srandom(timestamp);

	IF_NULL_RETURN(api_url, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_device_info, TC_IOT_NULL_POINTER);

    sign_len = tc_iot_create_auth_request_form(
        sign_out, sizeof(sign_out), p_device_info->secret,
        strlen(p_device_info->secret), p_device_info->client_id,
        strlen(p_device_info->client_id), p_device_info->device_name,
        strlen(p_device_info->device_name), expire, nonce,
        p_device_info->product_id, strlen(p_device_info->product_id),
        timestamp);

    LOG_TRACE("signed request form:\n%.*s", sign_len, sign_out);
    
    memset(&network, 0, sizeof(network));

    if (strncmp(api_url, HTTPS_PREFIX, HTTPS_PREFIX_LEN) == 0) {
#ifdef ENABLE_TLS
        tc_iot_tls_config_t* config;

        netcontext.fd = -1;
        netcontext.use_tls = 1;

        config = &(netcontext.tls_config);
        config->root_ca_in_mem = g_tc_iot_https_root_ca_certs;
        if (root_ca_path) {
            config->root_ca_location = root_ca_path;
        }
        config->timeout_ms = 10000;
        if (netcontext.use_tls) {
            config->verify_server = 1;
        }

        tc_iot_hal_tls_init(&network, &netcontext);
        /* init network end*/
        LOG_TRACE("tls network intialized.");
#else
        LOG_CRIT("tls network not supported.");
        return TC_IOT_TLS_NOT_SUPPORTED;
#endif
    } else {
        tc_iot_net_context_t netcontext;
        memset(&netcontext, 0, sizeof(netcontext));
        tc_iot_hal_net_init(&network, &netcontext);
        LOG_TRACE("dirtect tcp network intialized.");
    }

    /* request init begin */

    tc_iot_yabuffer_init(&request.buf, http_request_buffer,
                         sizeof(http_request_buffer));
    /* request init end */

    
    memset(http_resp, 0, sizeof(http_resp));
    ret = http_post_urlencoded(&network, &request, api_url, sign_out,
                                   http_resp, sizeof(http_resp), 2000);

    rsp_body = strstr(http_resp, "\r\n\r\n");
    if (rsp_body) {
        // skip \r\n\r\n

        int username_start = 0;
        int username_len = 0;

        int password_start = 0;
        int password_len = 0;

        int expire_start = 0;
        int expire_len = 0;

        jsmn_parser p;
        jsmntok_t t[20];
        jsmntok_t* temp;

        char temp_buf[256];
        int returnCodeIndex = 0;
        char num_buf[25];
        int expire_index ;
        long expire;
        int password_index;
        int r ;
        int username_index;

        jsmn_init(&p);

        rsp_body += 4;
        LOG_TRACE("\nbody:\n%s\n", rsp_body);
			
        r = jsmn_parse(&p, rsp_body, strlen(rsp_body), t, sizeof(t) / sizeof(t[0]));
        if (r < 0) {
            LOG_ERROR("Failed to parse JSON: %s", rsp_body);
            return TC_IOT_JSON_PARSE_FAILED;
        }

        if (r < 1 || t[0].type != JSMN_OBJECT) {
            LOG_ERROR("Invalid JSON: %s", rsp_body);
            return TC_IOT_JSON_PARSE_FAILED;
        }


        returnCodeIndex = tc_iot_json_find_token(rsp_body, t, r, "returnCode",
                                                    temp_buf,
                                                    sizeof(temp_buf));
        if (returnCodeIndex <= 0 || strlen(temp_buf) != 1 || temp_buf[0] != '0') {
            LOG_ERROR("failed to fetch token %d/%s: %s", returnCodeIndex, temp_buf, rsp_body);
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }

        username_index = tc_iot_json_find_token(rsp_body, t, r, "data.id",
                                                    p_device_info->username,
                                                    TC_IOT_MAX_USER_NAME_LEN);
        if (username_index <= 0) {
            LOG_TRACE("data.id not found in response.");
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }

        password_index = tc_iot_json_find_token(
            rsp_body, t, r, "data.secret", p_device_info->password,
            TC_IOT_MAX_PASSWORD_LEN);
        if (password_index <= 0) {
            LOG_TRACE("data.secret not found in response.");
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }


        expire_index = tc_iot_json_find_token(rsp_body, t, r, "data.expire",
                                                  num_buf, sizeof(num_buf));
        if (expire_index <= 0) {
            LOG_TRACE("data.expire not found in response.");
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }

        expire = atol(num_buf);
        p_device_info->token_expire_time = timestamp + expire;

        return TC_IOT_SUCCESS;
    } else {
        return ERROR_HTTP_REQUEST_FAILED;
    }
}

#ifdef __cplusplus
}
#endif
