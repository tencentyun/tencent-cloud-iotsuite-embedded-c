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

static void _trace_node(const char *prefix, const char *str, const jsmntok_t *node) {
    /* TC_IOT_LOG_TRACE("---%s type=%d,start=%d,end=%d,size=%d,parent=%d\t %s", */
    /* prefix,  */
    /* node->type, node->start, node->end, node->size, node->parent, */
    /* tc_iot_log_summary_string( str + node->start, node->end - node->start)); */
}

const char * tc_iot_json_token_type_str(int type) {
    switch(type) {
        case JSMN_UNDEFINED:
            return "undefined";
        case JSMN_OBJECT:
            return "object";
        case JSMN_ARRAY:
            return "array";
        case JSMN_STRING:
            return "string";
        case JSMN_PRIMITIVE:
            return "premitive:bool/number/null";
        default:
            return "unknown";
    }
}

void tc_iot_json_print_node(const char *prefix, const char *json, const jsmntok_t *root_node, int node_index) {
    const jsmntok_t * node;

	node = root_node + node_index;
    TC_IOT_LOG_TRACE("%s id=%d,type=%s,start=%d,end=%d,size(child_count)=%d,parent=%d\t %s",
    prefix, node_index,
    tc_iot_json_token_type_str(node->type), 
    node->start, node->end, node->size, node->parent,
    tc_iot_log_summary_string( json + node->start, node->end - node->start)
    );
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

    for (index = 0; src[index] && (index < src_len); index++) {
        if (src[index] == '\\') {
            valid_escaped = true;
            if (index < (src_len - 1)) {
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
                        if (src_len - index >= 5) {
                            ret = _unicode_char_to_long(&src[index + 2], 4,
                                                        &temp_unicode);
                            if (ret != TC_IOT_SUCCESS) {
                                valid_escaped = false;
                                TC_IOT_LOG_WARN("unicode data invalid %s",
                                         tc_iot_log_summary_string( &src[index], src_len - index));
                                break;
                            }
                            ret = tc_iot_unicode_to_utf8(&dest[dest_index],
                                                         dest_len - dest_index,
                                                         temp_unicode);
                            if (ret <= 0) {
                                valid_escaped = false;
                                TC_IOT_LOG_WARN(
                                    "unicode %ld transform to utf8 failed: "
                                    "ret=%d",
                                    temp_unicode, ret);
                                break;
                            }
                            dest_index += ret;
                            index += 5;
                        } else {
                                TC_IOT_LOG_WARN("unicode data invalid %s",
                                         tc_iot_log_summary_string( &src[index], src_len - index));
                        }
                        break;
                    default:
                        TC_IOT_LOG_WARN("invalid json escape:%s",
                                         tc_iot_log_summary_string( &src[index], src_len - index));
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

    return dest_index;
}

char *tc_iot_json_inline_escape(char *dest, int dest_len, const char *src) {
    tc_iot_json_escape(dest, dest_len, src, strlen(src));
    return dest;
}

int tc_iot_json_escape(char *dest, int dest_len, const char *src, int src_len) {
    int src_index;
    int dest_index;

    for (src_index = 0, dest_index = 0;
         src[src_index] && (src_index < src_len) && (dest_index < dest_len);
         src_index++) {
        switch (src[src_index]) {
            case '\b':
                dest[dest_index++] = '\\';
                dest[dest_index++] = 'b';
                break;
            case '\f':
                dest[dest_index++] = '\\';
                dest[dest_index++] = 'f';
                break;
            case '\n':
                dest[dest_index++] = '\\';
                dest[dest_index++] = 'n';
                break;
            case '\r':
                dest[dest_index++] = '\\';
                dest[dest_index++] = 'r';
                break;
            case '\t':
                dest[dest_index++] = '\\';
                dest[dest_index++] = 't';
                break;
            case '"':
            case '\\':
            case '/':
                dest[dest_index++] = '\\';
            default:
                dest[dest_index++] = src[src_index];
                break;
        }
    }

    if (dest_index <= (dest_len - 1)) {
        dest[dest_index] = '\0';
    }
    return dest_index;
}

int tc_iot_json_find_token(const char *json, const jsmntok_t *root_token,
                           int count, const char *path, char *result,
                           int result_len) {
    const char *name_start = path;
    int tok_index = 0;
    int parent_index = 0;
    int child_count;
    int visited_child;
    int token_name_len = 0;
    int val_len = 0;
    const char *pos;

    IF_NULL_RETURN(json, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(root_token, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(path, TC_IOT_NULL_POINTER);

    for (tok_index = 0; tok_index < count;) {
        pos = strstr(name_start, ".");
        if (NULL != pos) {
            token_name_len = pos - name_start;
        } else {
            token_name_len = strlen(name_start);
        }

        _trace_node("check node:", json, &root_token[tok_index]);
        if (root_token[tok_index].type != JSMN_OBJECT) {
            TC_IOT_LOG_ERROR("token %d not object", tok_index);
            return -1;
        }

        parent_index = tok_index;
        child_count = root_token[tok_index].size;
        tok_index++;
        visited_child = 0;

        for (; (visited_child < child_count) && (tok_index < count); tok_index++) {
            _trace_node("compare node:", json, &root_token[tok_index]);
            if (parent_index == root_token[tok_index].parent) {
                if (tc_iot_jsoneq_len(json, &root_token[tok_index], name_start,
                                      token_name_len) == 0) {
                    tok_index++;

                    if (NULL == pos) {
                        _trace_node("match node:", json,
                                    &root_token[tok_index]);

                        if (result && result_len) {
                            val_len = root_token[tok_index].end -
                                    root_token[tok_index].start;
                            if (val_len > result_len) {
                                return TC_IOT_BUFFER_OVERFLOW;
                            }

                            tc_iot_json_unescape(result, result_len,
                                    json + root_token[tok_index].start,
                                    val_len);
                            if (val_len < result_len) {
                                result[val_len] = 0;
                            }
                        }
                        /* TC_IOT_LOG_TRACE("result=%s", tc_iot_log_summary_string(result,val_len)); */
                        return tok_index;
                    }
                    break;
                } else {
                    _trace_node("node name not match:", json, &root_token[tok_index]); 
                }
                visited_child++;
            } else {
                /* TC_IOT_LOG_TRACE("target parent=%d/current parent=%d", parent_index, root_token[tok_index].parent); */
                _trace_node("node parent not match:", json, &root_token[tok_index]); 
            }
        }

        if (visited_child >= child_count) {
            TC_IOT_LOG_TRACE("%s no match in json.", path);
            return TC_IOT_JSON_PATH_NO_MATCH;
        }

        /* continue search */
        name_start = pos + 1;
        /* TC_IOT_LOG_TRACE("searching sub path: %s", name_start); */
    }

    return TC_IOT_JSON_PATH_NO_MATCH;
}

int tc_iot_json_parse(const char * json, int json_len, jsmntok_t * tokens, int token_count) {
    jsmn_parser p;
    int ret;

    jsmn_init(&p);
    ret = jsmn_parse(&p, json, json_len, tokens, token_count);

    if (ret < 0) {
        TC_IOT_LOG_ERROR("Failed to parse JSON: %s", tc_iot_log_summary_string(json, json_len));
        return TC_IOT_JSON_PARSE_FAILED;
    }

    if (ret < 1 || tokens[0].type != JSMN_OBJECT) {
        TC_IOT_LOG_ERROR("Failed to JSON format: %s", tc_iot_log_summary_string(json, json_len));
        return TC_IOT_JSON_PARSE_FAILED;
    }

    return ret;
}

DEFINE_TC_IOT_PROPERTY_FUNC(int8_t, TC_IOT_INT8);
DEFINE_TC_IOT_PROPERTY_FUNC(int16_t, TC_IOT_INT16);
DEFINE_TC_IOT_PROPERTY_FUNC(int32_t, TC_IOT_INT32);
DEFINE_TC_IOT_PROPERTY_FUNC(uint8_t, TC_IOT_UINT8);
DEFINE_TC_IOT_PROPERTY_FUNC(uint16_t, TC_IOT_UINT16);
DEFINE_TC_IOT_PROPERTY_FUNC(uint32_t, TC_IOT_UINT32);
DEFINE_TC_IOT_PROPERTY_FUNC(bool, TC_IOT_BOOL);
DEFINE_TC_IOT_PROPERTY_FUNC(float, TC_IOT_FLOAT);
DEFINE_TC_IOT_PROPERTY_FUNC(double, TC_IOT_DOUBLE);

tc_iot_property tc_iot_property_ref(const char *key, void *ptr,
                                    tc_iot_type_e type, int length) {
    tc_iot_property prop;
    prop.key = key;
    prop.length = length;
    prop.data.as_string = ptr;
    prop.type = type;
    return prop;
}

int _tc_iot_shadow_req_construct(char *buffer, int len, tc_iot_shadow_client *c,
                                 const char *method) {
    return snprintf(buffer, len, "{\"method\":\"%s\"}", method);
}

#ifdef __cplusplus
}
#endif
