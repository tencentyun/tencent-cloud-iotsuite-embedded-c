#include "tc_iot_inc.h"

int tc_iot_try_parse_int(const char * str, int * num_char_count) {
    int result = 0;
    int count = 0;

    if (num_char_count) {
        *num_char_count = 0;
    }

    if (!str) {
        return 0;
    }
    while(*str >= '0' && *str <= '9') {
        count++;
        result = result*10 + (*str - '0');
        str++;
    }

    if (num_char_count) {
        *num_char_count = count;
    }

    return result;
}

bool tc_iot_http_has_line_ended(const char * str) {
    while (*str) {
        if (*str == '\r') {
            return true;
        }
        str ++;
    }
    return false;
}

void tc_iot_http_parser_init(tc_iot_http_response_parser * parser) {
    if (parser) {
        parser->state = _PARSER_START;
        parser->version = 0;
        parser->status_code = 0;
        parser->content_length = 0;
        parser->location = NULL;
        /* parser->body = NULL; */
    }
}

int tc_iot_http_parser_analysis(tc_iot_http_response_parser * parser, const char * buffer, int buffer_len) {
    bool header_complete = false;
    const char * pos = NULL;
    int buffer_parsed = 0;
    int i = 0;

    IF_NULL_RETURN(parser, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);

    pos = buffer;
start:
    if (buffer_parsed >= buffer_len) {
        return buffer_parsed;
    }

    switch(parser->state) {
        case _PARSER_START:
            /* Head should alwarys start with: HTTP/1.x COD */
		    if ( buffer_len < sizeof("HTTP/1.x M0N")) {
                return 0;
            }

            if (!tc_iot_str7equal(pos, 'H', 'T', 'T', 'P', '/', '1', '.')) {
                TC_IOT_LOG_ERROR("Response header not start with HTTP/1.[x] .");
                return TC_IOT_FAILURE;
            } else {
                buffer_parsed += 7;
                pos = buffer + buffer_parsed;
            }

            if (pos[0] != '0' && pos[0] != '1') {
                TC_IOT_LOG_ERROR("HTTP version 1.%c not supported", pos[0]);
                return TC_IOT_FAILURE;
            } else {
                parser->version = pos[0] - '0';
            }

            if (' ' != pos[1]) {
                TC_IOT_LOG_ERROR("space not found");
                return TC_IOT_FAILURE;
            }

            buffer_parsed += 2;
            pos = buffer + buffer_parsed;
            parser->status_code = 0;
            for (i = 0; i < 3; i++) {
                if (pos[i] > '9' || pos[i] < '0') {
                    return TC_IOT_HTTP_INVALID_STATUS_CODE;
                } else {
                    parser->status_code = parser->status_code * 10 + pos[i] - '0';
                }
            }
            TC_IOT_LOG_TRACE("version: 1.%d, status code: %d",parser->version, parser->status_code);
            buffer_parsed += 3;
            pos = buffer + buffer_parsed;
            parser->state = _PARSER_IGNORE_TO_RETURN_CHAR;
            goto start;
        case _PARSER_IGNORE_TO_RETURN_CHAR:
            while (buffer_parsed < buffer_len) {
                if ('\r' == (*pos)) {
                    buffer_parsed++;
                    pos++;
                    parser->state = _PARSER_SKIP_NEWLINE_CHAR;
                    goto start;
                    break;
                }
                buffer_parsed++;
                pos++;
            }
            return buffer_parsed;
        case _PARSER_SKIP_NEWLINE_CHAR:
            if ('\n' != *pos) {
                TC_IOT_LOG_ERROR("expecting \\n");
                return TC_IOT_FAILURE;
            }
            buffer_parsed++;
            pos++;
            parser->state = _PARSER_HEADER;
            goto start;
        case _PARSER_HEADER:
            /* TC_IOT_LOG_TRACE("pos=%s",pos); */
            if ('\r' == (*pos)) {
                /* status line\r\n */
                /* http headers\r\n */
                /* \r\nbody */
                /* ^ */
                /* | we are here */
                buffer_parsed += 1;
                pos = buffer + buffer_parsed;
                parser->state = _PARSER_IGNORE_TO_BODY_START;
                goto start;
            } else {
                for (i = 0; i < (buffer_len-buffer_parsed); i++) {
                    if (':' == pos[i]) {
                        if ((i == tc_iot_const_str_len(HTTP_HEADER_CONTENT_LENGTH)) 
                                && (0 == memcmp(pos, HTTP_HEADER_CONTENT_LENGTH, i))) {
                            header_complete = tc_iot_http_has_line_ended(pos+i+1);
                            if (header_complete) {
                                /* TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_CONTENT_LENGTH, pos+i+2); */
                                parser->content_length = tc_iot_try_parse_int(pos+i+2, NULL);
                            } else {
                                TC_IOT_LOG_TRACE("%s not complete, continue reading:%s",HTTP_HEADER_CONTENT_LENGTH, pos+i+2);
                                return buffer_parsed;
                            }

                        } else if ((i == tc_iot_const_str_len(HTTP_HEADER_LOCATION)) 
                                && (0 == memcmp(pos, HTTP_HEADER_LOCATION, i))) {
                            header_complete = tc_iot_http_has_line_ended(pos+i+1);
                            if (header_complete) {
                                TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_LOCATION, pos+i+2);
                                parser->location = pos+i+2;
                            } else {
                                TC_IOT_LOG_TRACE("%s not complete, continue reading:%s",HTTP_HEADER_LOCATION, pos+i+2);
                                return buffer_parsed;
                            }
                        } else if ((i == tc_iot_const_str_len(HTTP_HEADER_CONTENT_TYPE)) 
                                && (0 == memcmp(pos, HTTP_HEADER_CONTENT_TYPE, i))) {
                            /* TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_CONTENT_TYPE, pos+i+2); */
                        } else {
                            /* TC_IOT_LOG_TRACE("ignore i=%d,pos=%s",i, pos); */
                        }
                        buffer_parsed += i+1;
                        pos = buffer + buffer_parsed;
                        parser->state = _PARSER_IGNORE_TO_RETURN_CHAR;
                        goto start;
                    }
                }
                TC_IOT_LOG_TRACE("buffer_parsed=%d, buffer_len=%d,left=%s", buffer_parsed, buffer_len, pos);
                return buffer_parsed;
            }
            break;
        case _PARSER_IGNORE_TO_BODY_START:
            if ('\n' != *pos) {
                TC_IOT_LOG_ERROR("expecting \\n");
                return TC_IOT_FAILURE;
            }
            buffer_parsed++;
            pos++;
            /* TC_IOT_LOG_TRACE("body=%s", pos); */
            parser->state = _PARSER_END;
            /* parser->body = pos; */
            return buffer_parsed;
        case _PARSER_END:
            return buffer_len;
        default:
            TC_IOT_LOG_ERROR("invalid parse state=%d", parser->state);
            return TC_IOT_FAILURE;
    }
}

int tc_iot_prepare_network(tc_iot_network_t * p_network, bool over_tls, const char * certs) {
    tc_iot_net_context_init_t netcontext;
#ifdef ENABLE_TLS
    tc_iot_tls_config_t* config;
#endif

    IF_NULL_RETURN(p_network, TC_IOT_NULL_POINTER);

    if (over_tls) {
#ifdef ENABLE_TLS
        netcontext.fd = -1;
        netcontext.use_tls = 1;

        config = &(netcontext.tls_config);
        config->root_ca_in_mem = certs;
        config->timeout_ms = 2000; // Default TLS read timeout
        if (netcontext.use_tls) {
            config->verify_server = 1;
        }

        tc_iot_hal_tls_init(p_network, &netcontext);
        TC_IOT_LOG_TRACE("tls network intialized.");
#else
        TC_IOT_LOG_FATAL("tls network not supported.");
        return TC_IOT_TLS_NOT_SUPPORTED;
#endif
    } else {
        netcontext.use_tls = 0;
        tc_iot_hal_net_init(p_network, &netcontext);
        TC_IOT_LOG_TRACE("dirtect tcp network intialized.");
    }
    return TC_IOT_SUCCESS;
}


int tc_iot_ota_download(const char* api_url, int partial_start, tc_iot_http_download_callback download_callback, const void * context) {
    tc_iot_network_t network;
    tc_iot_http_request request;
    unsigned char http_buffer[TC_IOT_HTTP_OTA_REQUEST_LEN];
    int max_http_resp_len = sizeof(http_buffer) - 1;
    char temp_buf[TC_IOT_HTTP_MAX_URL_LENGTH];
    int ret;
    int redirect_count = 0;
    int i = 0;
    int callback_ret = 0;
    int http_code = 0;
    int content_length = 0;
    int received_bytes = 0;
    int http_timeout_ms = 2000;
    tc_iot_http_response_parser parser;
    char http_header[32];
    int parse_ret = 0;
    int parse_left = 0;

    IF_NULL_RETURN(api_url, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(download_callback, TC_IOT_NULL_POINTER);

parse_url:

    memset(&network, 0, sizeof(network));

    TC_IOT_LOG_TRACE("request url=%s", api_url);
    if (strncmp(api_url, HTTPS_PREFIX, HTTPS_PREFIX_LEN) == 0) {
#if defined(ENABLE_TLS)
        tc_iot_prepare_network(&network, true, g_tc_iot_https_root_ca_certs);
#else
        TC_IOT_LOG_ERROR("TLS not enabled.");
#endif
    } else {
        tc_iot_prepare_network(&network, false, NULL);
    }

    tc_iot_yabuffer_init(&request.buf, (char *)http_buffer,
                         sizeof(http_buffer));

    TC_IOT_LOG_TRACE("request url=%s", api_url);
    if (partial_start > 0) {
        tc_iot_hal_snprintf(http_header, sizeof(http_header), "Range: bytes=%d-", partial_start);
    } else {
        http_header[0] = '\0';
    }
    ret = tc_iot_http_get(&network, &request, api_url,  http_timeout_ms, http_header);
    if (TC_IOT_SUCCESS != ret) {
        TC_IOT_LOG_ERROR("request url=%s failed, ret=%d", api_url, ret);
        return ret;
    }

    ret = network.do_read(&network, (unsigned char *)http_buffer, max_http_resp_len, http_timeout_ms);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("read from request url=%s failed, ret=%d", api_url, ret);
        return ret;
    }

    http_buffer[ret] = 0;
    tc_iot_http_parser_init(&parser);

    while (ret > 0) {
        parse_ret = tc_iot_http_parser_analysis(&parser, (char *)http_buffer, ret);
        if (parse_ret < 0) {
            TC_IOT_LOG_ERROR("read from request url=%s failed, ret=%d", api_url, ret);
            network.do_disconnect(&network);
            return parse_ret;
        }

        if (parse_ret > ret) {
            TC_IOT_LOG_ERROR("tc_iot_http_parser_analysis parse_ret=%d too large, ret=%d", parse_ret, ret);
            network.do_disconnect(&network);
            return TC_IOT_FAILURE;
        }

        parse_left = ret - parse_ret;
        if (parse_left > 0) {
            memmove(http_buffer, http_buffer+parse_ret, parse_left);
            http_buffer[parse_left] = '\0';
            /* TC_IOT_LOG_TRACE("buffer left:%s", http_buffer); */
        }
        
        if (301 == parser.status_code || 302 == parser.status_code) {
            TC_IOT_LOG_TRACE("server return redirect code=%d", parser.status_code);
            if (redirect_count < 5) {
                redirect_count++;
            } else {
                TC_IOT_LOG_ERROR("http code %d, redirect exceed maxcount=%d.", http_code, redirect_count);
                return TC_IOT_HTTP_REDIRECT_TOO_MANY;
            }

            if (parser.location) {
                TC_IOT_LOG_TRACE("new_url=%s",  parser.location);
                for (i = 0; i < ret; i++) {
                    temp_buf[i] = parser.location[i];
                    if (temp_buf[i] == '\r') {
                        TC_IOT_LOG_TRACE("truncate api url");
                        temp_buf[i] = '\0';
                    }
                    if (temp_buf[i] == '\0') {
                        break;
                    }
                }
                api_url = temp_buf;
                TC_IOT_LOG_TRACE("http response status code=%d, redirect times=%d, new_url=%s", 
                        parser.status_code, redirect_count, api_url);
            } else {
                TC_IOT_LOG_ERROR("http code %d, Location header not found.", ret);
            }

            goto parse_url;
        }

        if (parser.status_code != 200 && parser.status_code != 206) {
            TC_IOT_LOG_ERROR("http resoponse parser.status_code = %d", parser.status_code);
            network.do_disconnect(&network);
            return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
        }

        if (_PARSER_END == parser.state) {
            TC_IOT_LOG_TRACE("ver=1.%d, code=%d,content_length=%d", parser.version, parser.status_code, parser.content_length);
            content_length = parser.content_length;
            received_bytes = parse_left;

            callback_ret = download_callback(context, (const char *)http_buffer, received_bytes, 0, content_length);
            if (callback_ret != TC_IOT_SUCCESS) {
                TC_IOT_LOG_ERROR("callback failed ret=%d, abort.", callback_ret);
                return TC_IOT_FAILURE;
            }
            while( ret >= 0) {
                ret = network.do_read(&network, http_buffer, max_http_resp_len, http_timeout_ms);
                if ((ret <= max_http_resp_len) && (ret > 0)) {
                    http_buffer[ret] = '\0';
                    callback_ret = download_callback(context, (const char *)http_buffer, ret, received_bytes , content_length);
                    if (callback_ret != TC_IOT_SUCCESS) {
                        TC_IOT_LOG_ERROR("callback failed ret=%d, abort.", callback_ret);
                        return TC_IOT_FAILURE;
                    }
                    received_bytes += ret;

                    if (received_bytes >= content_length) {
                        TC_IOT_LOG_TRACE("%s=%d, received_bytes=%d", HTTP_HEADER_CONTENT_LENGTH, content_length, received_bytes);
                        network.do_disconnect(&network);
                        return TC_IOT_SUCCESS;
                    }
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
        }

        if (ret <= 0) {
            TC_IOT_LOG_TRACE("ret=%d, len=%d", ret, max_http_resp_len);
            network.do_disconnect(&network);
            return TC_IOT_SUCCESS;
        }

        ret = network.do_read(&network, (unsigned char *)http_buffer+parse_left,
                max_http_resp_len-parse_left, http_timeout_ms);
        if (ret >= 0) {
            ret += parse_left;
        }

    }

    return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
}

int tc_iot_ota_request_content_length(const char* api_url) {
    tc_iot_network_t network;
    tc_iot_http_request request;
    unsigned char http_buffer[TC_IOT_HTTP_OTA_REQUEST_LEN];
    int max_http_resp_len = sizeof(http_buffer) - 1;
    char temp_buf[TC_IOT_HTTP_MAX_URL_LENGTH];
    int ret;
    int redirect_count = 0;
    int i = 0;
    int http_code = 0;
    int http_timeout_ms = 2000;
    tc_iot_http_response_parser parser;
    int parse_ret = 0;
    int parse_left = 0;

    IF_NULL_RETURN(api_url, TC_IOT_NULL_POINTER);

parse_url:

    memset(&network, 0, sizeof(network));

    TC_IOT_LOG_TRACE("request url=%s", api_url);
    if (strncmp(api_url, HTTPS_PREFIX, HTTPS_PREFIX_LEN) == 0) {
#if defined(ENABLE_TLS)
        tc_iot_prepare_network(&network, true, g_tc_iot_https_root_ca_certs);
#else
        TC_IOT_LOG_ERROR("TLS not enabled.");
#endif
    } else {
        tc_iot_prepare_network(&network, false, NULL);
    }

    tc_iot_yabuffer_init(&request.buf, (char *)http_buffer,
                         sizeof(http_buffer));

    TC_IOT_LOG_TRACE("request url=%s", api_url);

    ret = tc_iot_http_head(&network, &request, api_url, http_timeout_ms);
    if (TC_IOT_SUCCESS != ret) {
        TC_IOT_LOG_ERROR("request url=%s failed, ret=%d", api_url, ret);
        return ret;
    }

    ret = network.do_read(&network, (unsigned char *)http_buffer, max_http_resp_len, http_timeout_ms);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("read from request url=%s failed, ret=%d", api_url, ret);
        return ret;
    }

    http_buffer[ret] = 0;
    tc_iot_http_parser_init(&parser);

    while (ret > 0) {
        parse_ret = tc_iot_http_parser_analysis(&parser, (const char *)http_buffer, ret);
        if (parse_ret < 0) {
            TC_IOT_LOG_ERROR("read from request url=%s failed, ret=%d", api_url, ret);
            network.do_disconnect(&network);
            return parse_ret;
        }

        if (parse_ret > ret) {
            TC_IOT_LOG_ERROR("tc_iot_http_parser_analysis parse_ret=%d too large, ret=%d", parse_ret, ret);
            network.do_disconnect(&network);
            return TC_IOT_FAILURE;
        }

        parse_left = ret - parse_ret;
        if (parse_left > 0) {
            memmove(http_buffer, http_buffer+parse_ret, parse_left);
            http_buffer[parse_left] = '\0';
        }
        
        if (301 == parser.status_code || 302 == parser.status_code) {
            TC_IOT_LOG_TRACE("server return redirect code=%d", parser.status_code);
            if (redirect_count < 5) {
                redirect_count++;
            } else {
                TC_IOT_LOG_ERROR("http code %d, redirect exceed maxcount=%d.", http_code, redirect_count);
                return TC_IOT_HTTP_REDIRECT_TOO_MANY;
            }

            if (parser.location) {
                TC_IOT_LOG_TRACE("new_url=%s",  parser.location);
                for (i = 0; i < ret; i++) {
                    temp_buf[i] = parser.location[i];
                    if (temp_buf[i] == '\r') {
                        TC_IOT_LOG_TRACE("truncate api url");
                        temp_buf[i] = '\0';
                    }
                    if (temp_buf[i] == '\0') {
                        break;
                    }
                }
                api_url = temp_buf;
                TC_IOT_LOG_TRACE("http response status code=%d, redirect times=%d, new_url=%s", 
                        ret, redirect_count, api_url);
            } else {
                TC_IOT_LOG_ERROR("http code %d, Location header not found.", ret);
            }

            goto parse_url;
        }

        if (parser.status_code != 200 && parser.status_code != 206) {
            TC_IOT_LOG_ERROR("http resoponse parser.status_code = %d", parser.status_code);
            network.do_disconnect(&network);
            return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
        }

        if (parser.content_length > 0) {
            TC_IOT_LOG_TRACE("ver=1.%d, code=%d,content_length=%d", parser.version, parser.status_code, parser.content_length);
            network.do_disconnect(&network);
            return parser.content_length;
        }

        if (ret <= 0) {
            TC_IOT_LOG_TRACE("ret=%d, len=%d", ret, max_http_resp_len);
            return TC_IOT_SUCCESS;
        }

        ret = network.do_read(&network, (unsigned char *)http_buffer+parse_left,
                max_http_resp_len-parse_left, http_timeout_ms);
        if (ret >= 0) {
            ret += parse_left;
        }
    }

    return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
}


