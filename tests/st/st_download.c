#include "tc_iot_export.h"

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"
#define HTTPS_PREFIX "https"
#define HTTPS_PREFIX_LEN (sizeof(HTTPS_PREFIX) - 1)

int http_get(tc_iot_network_t* network,
                         tc_iot_http_request* request, const char* url,
                         char* resp, int resp_max_len,
                         int timeout_ms) {
    tc_iot_url_parse_result_t result;
    char temp_host[512];
    int written_len;
    int read_len;
    int i = 0;
    int ret = tc_iot_url_parse(url, strlen(url), &result);

    if (ret < 0) {
        return ret;
    }

    if (result.path_len) {
        ret = tc_iot_create_get_request(
            request, url + result.path_start, result.path_len,
            url + result.host_start, result.host_len);
    } else {
        ret =
            tc_iot_create_get_request(request, "/", 1, url + result.host_start,
                                       result.host_len);
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

    TC_IOT_LOG_TRACE("remote=%s:%d", temp_host, result.port);

    network->do_connect(network, temp_host, result.port);
    written_len = network->do_write(network, (unsigned char *)request->buf.data,
                                    request->buf.pos, timeout_ms);
    TC_IOT_LOG_TRACE("request with:\n%s", request->buf.data);
    tc_iot_hal_printf("response with:\n");
    tc_iot_hal_printf("________________________\n");

    read_len = network->do_read(network, (unsigned char *)resp, resp_max_len-1, timeout_ms);
    while (read_len >= resp_max_len-1) {
        tc_iot_hal_printf("%s", resp);
        read_len = network->do_read(network, (unsigned char *)resp, resp_max_len-1, timeout_ms);
    }

    if (read_len > 0) {
        resp[read_len] = '\0';
        tc_iot_hal_printf("%s", resp);
    }
    tc_iot_hal_printf("\n________________________\n");

    network->do_disconnect(network);

    return read_len;
}

int do_download(const char* api_url) {
    tc_iot_network_t network;
    tc_iot_http_request request;
    unsigned char http_request_buffer[2048];
    char http_resp[512];
    char temp_buf[128];
    int ret;
    char* rsp_body;
    int redirect_count = 0;
    int temp_len = 0;
    int i = 0;
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

    memset(http_resp, 0, sizeof(http_resp));
    TC_IOT_LOG_TRACE("request url=%s", api_url);
    ret = http_get(&network, &request, api_url, http_resp, sizeof(http_resp), 2000);

    ret = tc_iot_parse_http_response_code(http_resp);
    if (ret != 200) {
        if (ret == 301 || ret == 302) {

            if (redirect_count < 5) {
                redirect_count++;
            } else {
                TC_IOT_LOG_ERROR("http code %d, redirect exceed maxcount=%d.", ret, redirect_count);
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
                TC_IOT_LOG_TRACE("http response status code=%d, redirect times=%d, new_url=%s", ret, redirect_count, api_url);
            } else {
                TC_IOT_LOG_ERROR("http code %d, Location header not found.", ret);
            }

            goto parse_url;
        } else {
            TC_IOT_LOG_WARN("http response status code=%d", ret);
        }
        return TC_IOT_REFRESH_TOKEN_FAILED;
    }

    rsp_body = strstr(http_resp, "\r\n\r\n");
    if (rsp_body) {
        rsp_body += 4;
        TC_IOT_LOG_TRACE("\nbody:\n%s\n", rsp_body);
        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
    }
}


int main(int argc, char** argv) {
    int ret = 0;
    /* const char * download_url = "http://localhost/test.dat"; */
    /* const char * download_url = "http://iot.devhost/test.tar"; */
    const char * download_url = "http://iot.devhost/test.dat";
    do_download(download_url);
}


static volatile int stop = 0;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop ++;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop ++;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
    if (stop >= 3) {
        tc_iot_hal_printf("SIGINT/SIGTERM received over %d times, force shutdown now.\n", stop);
        exit(0);
    }
}


