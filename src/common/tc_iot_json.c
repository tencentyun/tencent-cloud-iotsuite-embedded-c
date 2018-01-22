#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    IF_NULL_RETURN(json, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(tok, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(s, TC_IOT_NULL_POINTER);
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

static void _trace_node(const char *prefix, const char *str,
                        const jsmntok_t *node) {
    return;
    LOG_TRACE("---%s type=%d,start=%d,end=%d,size=%d,parent=%d\t %.*s", prefix,
              node->type, node->start, node->end, node->size, node->parent,
              node->end - node->start, str + node->start);
}

int tc_iot_jsoneq_len(const char *json, const jsmntok_t *tok, const char *s,
                      int len) {
    IF_NULL_RETURN(json, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(tok, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(s, TC_IOT_NULL_POINTER);
    if (tok->type == JSMN_STRING && (int)len == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

int _unicode_char_to_long(const char *unicode_str, int len,
                          unsigned long *code) {
    int i = 0;
    unsigned long result = 0;
    unsigned char temp = 0;

    IF_NULL_RETURN(unicode_str, TC_IOT_NULL_POINTER);
    IF_LESS_RETURN(8, len, TC_IOT_INVALID_PARAMETER);

    for (i = 0; i < len; i++) {
        temp = unicode_str[i];
        /* transform hex character to the 4bit equivalent number, using the
         * ascii table indexes */
        if (temp >= '0' && temp <= '9') {
            temp = temp - '0';
        } else if (temp >= 'a' && temp <= 'f') {
            temp = temp - 'a' + 10;
        } else if (temp >= 'A' && temp <= 'F') {
            temp = temp - 'A' + 10;
        } else {
            return TC_IOT_INVALID_PARAMETER;
        }
        result = (result << 4) | (temp & 0xF);
    }

    *code = result;
    return TC_IOT_SUCCESS;
}

int tc_iot_unicode_to_utf8(char *output, int output_len, unsigned long code) {
    IF_NULL_RETURN(output, TC_IOT_NULL_POINTER);
    if (code <= 0x7F) {
        IF_LESS_RETURN(output_len, 1, TC_IOT_BUFFER_OVERFLOW);
        /* * U-00000000 - U-0000007F:  0xxxxxxx */
        *output = (code & 0x7F);
        return 1;
    } else if (code >= 0x80 && code <= 0x7FF) {
        IF_LESS_RETURN(output_len, 2, TC_IOT_BUFFER_OVERFLOW);

        /* * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx */
        *(output + 1) = (code & 0x3F) | 0x80;
        *output = ((code >> 6) & 0x1F) | 0xC0;
        return 2;
    } else if (code >= 0x800 && code <= 0xFFFF) {
        IF_LESS_RETURN(output_len, 3, TC_IOT_BUFFER_OVERFLOW);

        /* * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx */
        *(output + 2) = (code & 0x3F) | 0x80;
        *(output + 1) = ((code >> 6) & 0x3F) | 0x80;
        *output = ((code >> 12) & 0x0F) | 0xE0;
        return 3;
    } else if (code >= 0x10000 && code <= 0x1FFFFF) {
        IF_LESS_RETURN(output_len, 4, TC_IOT_BUFFER_OVERFLOW);

        /* * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        *(output + 3) = (code & 0x3F) | 0x80;
        *(output + 2) = ((code >> 6) & 0x3F) | 0x80;
        *(output + 1) = ((code >> 12) & 0x3F) | 0x80;
        *output = ((code >> 18) & 0x07) | 0xF0;
        return 4;
    } else if (code >= 0x200000 && code <= 0x3FFFFFF) {
        IF_LESS_RETURN(output_len, 5, TC_IOT_BUFFER_OVERFLOW);

        /* * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx
         * 10xxxxxx */
        *(output + 4) = (code & 0x3F) | 0x80;
        *(output + 3) = ((code >> 6) & 0x3F) | 0x80;
        *(output + 2) = ((code >> 12) & 0x3F) | 0x80;
        *(output + 1) = ((code >> 18) & 0x3F) | 0x80;
        *output = ((code >> 24) & 0x03) | 0xF8;
        return 5;
    } else if (code >= 0x4000000 && code <= 0x7FFFFFFF) {
        IF_LESS_RETURN(output_len, 6, TC_IOT_BUFFER_OVERFLOW);

        /* * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx
         * 10xxxxxx 10xxxxxx */
        *(output + 5) = (code & 0x3F) | 0x80;
        *(output + 4) = ((code >> 6) & 0x3F) | 0x80;
        *(output + 3) = ((code >> 12) & 0x3F) | 0x80;
        *(output + 2) = ((code >> 18) & 0x3F) | 0x80;
        *(output + 1) = ((code >> 24) & 0x3F) | 0x80;
        *output = ((code >> 30) & 0x01) | 0xFC;
        return 6;
    }

    return 0;
}

int tc_iot_json_unescape(char *dest, int dest_len, const char *src,
                         int src_len) {
    int index = 0;
    int ret;
    int dest_index = 0;
    bool valid_escaped = true;
    unsigned long temp_unicode;
    int min_len = src_len < dest_len ? src_len : dest_len;

    for (index = 0; src[index] && (index < min_len); index++) {
        if (src[index] == '\\') {
            valid_escaped = true;
            if (index < (min_len - 1)) {
                switch (src[index + 1]) {
                    case '"':
                        dest[dest_index++] = '"';
                        index++;
                        break;
                    case '\\':
                        dest[dest_index++] = '\\';
                        index++;
                        break;
                    case '/':
                        dest[dest_index++] = '/';
                        index++;
                        break;
                    case 'b':
                        dest[dest_index++] = '\b';
                        index++;
                        break;
                    case 'f':
                        dest[dest_index++] = '\f';
                        index++;
                        break;
                    case 'n':
                        dest[dest_index++] = '\n';
                        index++;
                        break;
                    case 'r':
                        dest[dest_index++] = '\r';
                        index++;
                        break;
                    case 't':
                        dest[dest_index++] = '\t';
                        index++;
                        break;
                    case 'u':
                        if (min_len - index >= 5) {
                            ret = _unicode_char_to_long(&src[index + 2], 4,
                                                        &temp_unicode);
                            if (ret != TC_IOT_SUCCESS) {
                                valid_escaped = false;
                                LOG_WARN("unicode data invalid %.*s",
                                         min_len - index, &src[index]);
                                break;
                            }
                            ret = tc_iot_unicode_to_utf8(&dest[dest_index],
                                                         dest_len - dest_index,
                                                         temp_unicode);
                            if (ret <= 0) {
                                valid_escaped = false;
                                LOG_WARN(
                                    "unicode %ld transform to utf8 failed: "
                                    "ret=%d",
                                    temp_unicode, ret);
                                break;
                            }
                            dest_index += ret;
                        }
                        break;
                    default:
                        LOG_WARN("invalid json escape:%.*s", min_len - index,
                                 &src[index]);
                        valid_escaped = false;
                        break;
                }
            }

            if (valid_escaped) {
                continue;
            }
        }
        dest[dest_index++] = src[index];
    }
    if (index < dest_len) {
        dest[dest_index] = '\0';
    }
}

int tc_iot_json_find_token(const char *json, const jsmntok_t *root_token,
                           int count, const char *path, char *result,
                           int result_len) {
    IF_NULL_RETURN(json, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(root_token, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(path, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(result, TC_IOT_NULL_POINTER);

    const char *name_start = path;
    int tok_index = 0;
    int parent_index = 0;
    int j;
    int child_count;
    int visited_child;
    int token_name_len = 0;
    char is_last = 0;
    int val_len = 0;

    for (tok_index = 0; tok_index < count;) {
        const char *pos = strstr(name_start, ".");
        if (NULL != pos) {
            token_name_len = pos - path;
        } else {
            token_name_len = strlen(name_start);
        }

        _trace_node("check node:", json, &root_token[tok_index]);
        if (root_token[tok_index].type != JSMN_OBJECT) {
            LOG_ERROR("token %d not object", tok_index);
            return -1;
        }
        parent_index = tok_index;
        child_count = root_token[tok_index].size;
        tok_index++;
        visited_child = 0;

        for (; (visited_child < child_count) && (tok_index < count);
             tok_index++) {
            _trace_node("compare node:", json, &root_token[tok_index]);
            if (parent_index == root_token[tok_index].parent) {
                if (tc_iot_jsoneq_len(json, &root_token[tok_index], name_start,
                                      token_name_len) == 0) {
                    tok_index++;

                    if (NULL == pos) {
                        _trace_node("match node:", json,
                                    &root_token[tok_index]);

                        val_len = root_token[tok_index].end -
                                  root_token[tok_index].start;
                        if (val_len > result_len) {
                            return -1;
                        }

                        tc_iot_json_unescape(result, result_len,
                                             json + root_token[tok_index].start,
                                             val_len);
                        if (val_len < result_len) {
                            result[val_len] = 0;
                        }
                        LOG_TRACE("result=%.*s", val_len, result);
                        return tok_index;
                    }
                    break;
                } else {
                    _trace_node("node name not match:", json,
                                &root_token[tok_index]);
                }
                visited_child++;
            } else {
                _trace_node("node parent not match:", json,
                            &root_token[tok_index]);
            }
        }

        if (visited_child >= child_count) {
            LOG_TRACE("%s no match in json.", path);
            return TC_IOT_JSON_PATH_NO_MATCH;
        }

        /* continue search */
        name_start = pos + 1;
        LOG_TRACE("searching sub path: %s", name_start);
    }
}

#ifdef __cplusplus
}
#endif
