#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

static unsigned char hexchars[] = "0123456789ABCDEF";

static int _hexchr2int(char ch) {
    int val = ch;
    if ((val >= 'A') && (val <= 'F')) {
        return (val - 'A' + 10);
    }

    if ((val >= 'a') && (val <= 'f')) {
        return (val - 'a' + 10);
    }

    if ((val >= '0') && (val <= '9')) {
        return (val - '0');
    }

    /* invalid input comes in!!! */
    return 0;
}

static int _htoi(const char *s) {
    int value = _hexchr2int(s[0]) * 16 + _hexchr2int(s[1]);
    return (value);
}

static int is_normal_url_char(char c) {
    int result = ((c < '0' && c != '-' && c != '.') || (c < 'A' && c > '9') ||
                  (c > 'Z' && c < 'a' && c != '_') || (c > 'z'));
    return result;
}

/* max_output_len nomally equals input_len*3+1 */
int tc_iot_url_encode(const char *input, int input_len, char *output,
                      int max_output_len) {
    unsigned char ch;
    int pos = 0;
    unsigned char const *from, *end;

    from = (unsigned char *)input;
    end = (unsigned char *)input + input_len;

    while ((from < end) && (pos < max_output_len)) {
        ch = *from++;

        if (ch == ' ') {
            output[pos] = '+';
            pos++;
        } else if (is_normal_url_char(ch)) {
            output[pos] = '%';
            pos++;
            output[pos] = hexchars[ch >> 4];
            pos++;
            output[pos] = hexchars[ch & 15];
            pos++;
        } else {
            output[pos] = ch;
            pos++;
        }
    }
    if (pos < max_output_len) {
        output[pos] = '\0';
    }
    return pos;
}

int tc_iot_url_decode(const char *input, int input_len, char *output,
                      int max_output_len) {
    int pos = 0;
    const char *data = input;
    int left_len = input_len;

    while ((left_len--) && (pos < max_output_len)) {
        if (*data == '+') {
            output[pos] = ' ';
            pos++;
        } else if (*data == '%' && left_len >= 2 &&
                   isxdigit((int)*(data + 1)) && isxdigit((int)*(data + 2))) {
            output[pos] = (char)_htoi(data + 1);
            pos += 2;
            left_len -= 2;
        } else {
            output[pos] = *data;
            pos++;
        }
        data++;
    }
    if (pos < max_output_len) {
        output[pos] = '\0';
    }
    return pos;
}

/* accoding to rfc: */
/*                      hierarchical part */
/*          ┌───────────────────┴─────────────────────┐ */
/*                      authority               path */
/*          ┌───────────────┴───────────────┐┌───┴────┐ */
/*    abc://username:password@example.com:123/path/data?key=value&key2=value2#fragid1
 */
/*    └┬┘   └───────┬───────┘ └────┬────┘ └┬┘           └─────────┬─────────┘
 * └──┬──┘ */
/*  scheme  user information     host     port                  query
 * fragment */
/*   */
/*    urn:example:mammal:monotreme:echidna */
/*    └┬┘ └──────────────┬───────────────┘ */
/*  scheme              path */
/* */
/*  we only support currently: */
/*  http(s)://host[:port][/path][?query][#fragment] */
/* */
int tc_iot_url_parse(const char *input_url, int input_url_len,
                     tc_iot_url_parse_result_t *result) {
    int pos = 0;
    int prefix_len;
    int split_len;

    /* at least should be http://x */
    if (input_url_len <= URL_LEAST_LEN) {
        TC_IOT_LOG_ERROR("url invalid: %s", tc_iot_log_summary_string(input_url, input_url_len));
        return -1;
    }

    /* check if prefix with 'http' */
    prefix_len = sizeof(HTTP_SCHEME_PREFIX) - 1;
    if (strncmp(HTTP_SCHEME_PREFIX, &input_url[pos], prefix_len) != 0) {
        TC_IOT_LOG_ERROR("url invalid: %s", tc_iot_log_summary_string(input_url, input_url_len));
        return -1;
    }
    pos += prefix_len;

    /* check https scheme */
    if (input_url[pos] == 's') {
        result->over_tls = 1;
        result->port = HTTPS_DEFAULT_PORT;
        result->scheme_len = sizeof(HTTPS_SCHEME_PREFIX) - 1;
        pos++;
    } else {
        result->over_tls = 0;
        result->scheme_len = sizeof(HTTP_SCHEME_PREFIX) - 1;
        result->port = HTTP_DEFAULT_PORT;
    }

    /* check '://' */
    split_len = sizeof(SCHEME_SPLIT_STR) - 1;
    if (strncmp(SCHEME_SPLIT_STR, &input_url[pos], split_len) != 0) {
        TC_IOT_LOG_ERROR("url invalid: %s", input_url);
        return -1;
    }
    pos += split_len;

    result->host_start = pos;
    /* set default host len in case of url is 'http://xx.y.com' with port or
     * paht */
    result->host_len = input_url_len - pos;

    /* finding the host port and path(abs_path?query_string) */
    for (; pos < input_url_len; pos++) {
        /* meet custom port */
        if (':' == input_url[pos]) {
            int temp_port = 0;
            result->host_len = pos - (result->host_start);

            pos++;
            while ((input_url[pos] >= '0') && (input_url[pos] <= '9') &&
                   (pos < input_url_len)) {
                temp_port = temp_port * 10 + (input_url[pos] - '0');
                pos++;
            }
            result->port = temp_port;
            if (pos >= input_url_len) {
                result->path_start = input_url[pos - 1];
                result->path_len = 0;
                TC_IOT_LOG_TRACE(
                    "host_start=%d,host_len=%d,path_start=%d,path_len=%d,port=%"
                    "d",
                    result->host_start, result->host_len, result->path_start,
                    result->path_len, result->port);
                return 0;
            } else if ('/' != input_url[pos]) {
                /* port should follow with '/', thing goes wrong if not. */
                TC_IOT_LOG_ERROR("url invalid: %s", input_url);
                return -1;
            }
        }

        /* find abs_path and query_string or else followed */
        if ('/' == input_url[pos]) {
            result->host_len = pos - (result->host_start);
            result->path_start = pos;
            result->path_len = input_url_len - pos;
            break;
        }
    }

    TC_IOT_LOG_TRACE("host_start=%d,host_len=%d,path_start=%d,path_len=%d,port=%d",
              result->host_start, result->host_len, result->path_start,
              result->path_len, result->port);

    return 0;
}

#ifdef __cplusplus
}
#endif
