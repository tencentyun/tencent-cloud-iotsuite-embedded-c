#include "tc_iot_inc.h"

unsigned short tc_iot_coap_get_next_pack_id(tc_iot_coap_client* c) {
    if (!c) {
        return 0;
    }
    return c->next_packetid =
               (c->next_packetid == TC_IOT_COAP_MAX_MESSAGE_ID) ? 1 : c->next_packetid + 1;
}

int tc_iot_coap_write_char(unsigned char * buffer, int buffer_len, unsigned char val) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    if (buffer_len < sizeof(val)) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }
    *(unsigned char *)buffer = val;
    return sizeof(val);
}

int tc_iot_coap_write_short(unsigned char * buffer, int buffer_len, unsigned short val) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    if (buffer_len < sizeof(val)) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }
    *(unsigned short *)buffer = htons(val);
    return sizeof(val);
}

int tc_iot_coap_write_int(unsigned char * buffer, int buffer_len, unsigned int val) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    if (buffer_len < sizeof(val)) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    *(unsigned int *)buffer = htonl(val);
    return sizeof(val);
}


int tc_iot_coap_write_bytes(unsigned char * buffer, int buffer_len, const unsigned char * bytes, int bytes_len) {
    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    if (buffer_len < bytes_len) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    memcpy(buffer, bytes, bytes_len);
    return bytes_len;
}

unsigned int tc_iot_coap_extendable_number_base(unsigned int number) {
    if (number <= 12)  {
        return number;
    } else if (number > 12 && number <= 13 + 255) {
        return 13;
    } else if (number >= 14 + 255 && number <= 14 + 65535) {
        return 14;
    } else {
        TC_IOT_LOG_ERROR("number out of range: %d", number)
        return 0;
    }
}

unsigned int tc_iot_coap_extendable_number_extra_len(unsigned int number) {
    if (number <= 12)  {
        return 0;
    } else if (number > 12 && number <= 13 + 255) {
        return 1;
    } else if (number >= 14 + 255 && number <= 14 + 65535) {
        return 2;
    } else {
        TC_IOT_LOG_ERROR("number out of range: %d", number)
        return 0;
    }
}

unsigned int tc_iot_coap_extendable_number_extra_data(unsigned int number) {
    if (number <= 12)  {
        return 0;
    } else if (number >= 13 && number <= 13 + 255) {
        return number - 13;
    } else if (number >= 14 + 255 && number <= 14 + 65535) {
        return number - 14 - 255;
    } else {
        TC_IOT_LOG_ERROR("number out of range: %d", number)
        return 0;
    }

}

int tc_iot_coap_write_option(unsigned char * buffer, int buffer_len, unsigned int delta, unsigned int length, unsigned char * value) {
    unsigned char option_base ;
    int pos = 0;
    int extended_len = 0;

    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    if (buffer_len < (length+1)) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    option_base = ((tc_iot_coap_extendable_number_base(delta) & 0xF) << 4);
    option_base |= (tc_iot_coap_extendable_number_base(length) & 0xF);

    buffer[pos] = option_base;
    pos++;

    extended_len = tc_iot_coap_extendable_number_extra_len(delta);
    if (extended_len == 1) {
        *(unsigned char *)(buffer+pos) = tc_iot_coap_extendable_number_extra_data(delta);
    } else if (extended_len == 2){
        *(unsigned short *)(buffer+pos) = htons(tc_iot_coap_extendable_number_extra_data(delta));
    } else {
        if (extended_len != 0) {
            TC_IOT_LOG_ERROR("delta=%d, extended_len = %d invalid, should be 0~2", delta, extended_len);
        }
    }

    pos += extended_len;
    if (pos+length > buffer_len) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    extended_len = tc_iot_coap_extendable_number_extra_len(length);
    if (extended_len == 1) {
        *(unsigned char *)(buffer+pos) = tc_iot_coap_extendable_number_extra_data(length);
    } else if (extended_len == 2){
        *(unsigned short *)(buffer+pos) = htons(tc_iot_coap_extendable_number_extra_data(length));
    } else {
        if (extended_len != 0) {
            TC_IOT_LOG_ERROR("length = %d, extended_len = %d invalid, should be 0~2", length, extended_len);
        }
    }

    pos += extended_len;
    if (pos+length > buffer_len) {
        TC_IOT_LOG_ERROR("buffer overflow ");
        return TC_IOT_BUFFER_OVERFLOW;
    }
    memcpy(buffer+pos, value, length);
    pos += length;

    return pos;
}

int tc_iot_coap_serialize(unsigned char * buffer, int buffer_len, const tc_iot_coap_message * message) {
    int i = 0;
    int pos = 0;
    int ret = 0;
    int delta = 0;

    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    if (NULL == message){
        TC_IOT_LOG_ERROR("message is null");
        return TC_IOT_NULL_POINTER;
    }

    ret = tc_iot_coap_write_char(buffer+pos, buffer_len+pos, message->header.all);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("buffer overflow after write header=0x%X", (int)message->header.all);
        return ret;
    }
    pos += ret;

    ret = tc_iot_coap_write_char(buffer+pos, buffer_len+pos, message->code);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("buffer overflow after write code=%s", tc_iot_coap_get_message_code_str(message->code));
        return ret;
    }
    pos += ret;

    ret = tc_iot_coap_write_short(buffer+pos, buffer_len+pos, message->message_id);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("buffer overflow after write message id=%d", message->message_id);
        return ret;
    }
    pos += ret;

    ret = tc_iot_coap_write_bytes(buffer+pos, buffer_len+pos, message->token, message->header.bits.token_len);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("buffer overflow after write token, len=%d", message->header.bits.token_len);
        return ret;
    }
    pos += ret;

    tc_iot_coap_option * current = NULL;
    for (i = 0; i < message->option_count; i++) {
        if (i == 0) {
            delta = message->options[i].number;
        } else {
            delta = message->options[i].number - message->options[i-1].number;
        }
        ret = tc_iot_coap_write_option(buffer+pos, buffer_len-pos, delta, message->options[i].length, message->options[i].value);
        if (ret < 0) {
            TC_IOT_LOG_ERROR("buffer overflow after write Option=%s", tc_iot_coap_get_option_number_str(message->options[i].number));
            return ret;
        }
        pos += ret;
    }

    if (message->payload_len && message->p_payload) {
        ret = tc_iot_coap_write_char(buffer+pos, buffer_len+pos, TC_IOT_COAP_PAYLOAD_MARKER);
        if (ret < 0) {
            TC_IOT_LOG_ERROR("buffer overflow after write payload marker 0xFF");
            return ret;
        }
        pos += ret;

        ret = tc_iot_coap_write_bytes(buffer+pos, buffer_len+pos, message->p_payload, message->payload_len);
        if (ret < 0) {
            TC_IOT_LOG_ERROR("buffer overflow after write payload, payload_len=%d", message->payload_len);
            return ret;
        }
        pos += ret;
    } else {
        tc_iot_coap_message_dump(message);
    }

    return pos;
}


void tc_iot_coap_message_dump(const tc_iot_coap_message * message) {
    int i = 0;

    if (NULL == message){
        TC_IOT_LOG_ERROR("message is null");
        return ;
    }

    TC_IOT_LOG_TRACE("ver=%d,type=%s,tkl=%d,code=%s,message_id=%d",
            message->header.bits.ver, tc_iot_coap_get_message_type_str(message->header.bits.type),
            message->header.bits.token_len, tc_iot_coap_get_message_code_str(message->code), message->message_id);

    for (i = 0; i < message->option_count; i++) {
        if (COAP_OPTION_URI_PATH) {
            TC_IOT_LOG_TRACE("option number=%s,value=%s", tc_iot_coap_get_option_number_str(message->options[i].number), message->options[i].value);
        } else {
            TC_IOT_LOG_TRACE("option number=%s,length=%d", tc_iot_coap_get_option_number_str(message->options[i].number), message->options[i].length);
        }
    }
    
    if (message->payload_len && message->p_payload) {
        if (strlen(message->p_payload) == message->payload_len) {
            TC_IOT_LOG_TRACE("payload_len=%d,payload=%s",message->payload_len,message->p_payload);
        } else {
            TC_IOT_LOG_TRACE("payload_len=%d,payload=[binary data ...]",message->payload_len);
        }
    } else {
        TC_IOT_LOG_TRACE("payload_len=%d,no payload",message->payload_len);
    }
}


int tc_iot_coap_deserialize(tc_iot_coap_message * message, unsigned char * buffer, int buffer_len) {
    int i = 0;
    int pos = 0;
    int ret = 0;
    int delta = 0;
    int sum_delta = 0;
    int length = 0;
    unsigned char option_base = 0;

    if (NULL == buffer){
        TC_IOT_LOG_ERROR("buffer is null");
        return TC_IOT_NULL_POINTER;
    }

    if (NULL == message){
        TC_IOT_LOG_ERROR("message is null");
        return TC_IOT_NULL_POINTER;
    }

    if (buffer_len < 4){
        TC_IOT_LOG_ERROR("message is null");
        return TC_IOT_NULL_POINTER;
    }

    message->header.all = buffer[pos];
    pos++;
    message->code = buffer[pos];
    pos++;
    message->message_id = ntohs(*(unsigned short *) (buffer+pos));
    pos += 2;
    message->p_payload = NULL;
    message->payload_len = 0;

    TC_IOT_LOG_TRACE("received: ver=%d,type=%s,tkl=%d,code=%s,message_id=%d",
            message->header.bits.ver, tc_iot_coap_get_message_type_str(message->header.bits.type),
            message->header.bits.token_len, tc_iot_coap_get_message_code_str(message->code), message->message_id);

    if (message->header.bits.token_len > 0) {
        memcpy(message->token,buffer+pos, message->header.bits.token_len);
        pos += message->header.bits.token_len;
    }

    if (pos >= buffer_len) {
        TC_IOT_LOG_TRACE("no more data after token:pos=%d,buffer_len=%d", pos, buffer_len);
        return TC_IOT_SUCCESS;
    }

    while (pos < buffer_len) {
        option_base = buffer[pos];
        pos++;
        if (TC_IOT_COAP_PAYLOAD_MARKER == option_base) {
            break;
        }
        delta = (option_base >> 4) & 0xF;
        length = option_base & 0xF;
        /* TC_IOT_LOG_TRACE("delta=%d, length=%d", delta, length); */
        if (delta == 13) {
            delta += *(unsigned char *)(buffer+pos);
            pos++;
            TC_IOT_LOG_TRACE("real delta=%d", delta);
        } else if (delta == 14) {
            delta += ntohs(*(unsigned short *)(buffer+pos));
            TC_IOT_LOG_TRACE("real delta=%d", delta);
            pos += 2;
        } else if (delta == 15) {
            TC_IOT_LOG_ERROR("delta=%d is not acceptable", delta);
            return TC_IOT_COAP_MSG_OPTION_DELTA_INVALID;
        }

        sum_delta += delta;
        TC_IOT_LOG_TRACE("Option=%s",tc_iot_coap_get_option_number_str(sum_delta));

        if (length == 13) {
            length += *(unsigned char *)(buffer+pos);
            pos++;
            TC_IOT_LOG_TRACE("real length=%d", length);
        } else if (length == 14) {
            length += ntohs(*(unsigned short *)(buffer+pos));
            TC_IOT_LOG_TRACE("real length=%d", length);
            pos += 2;
        } else if (length == 15) {
            TC_IOT_LOG_ERROR("length=%d is not acceptable", length);
            return TC_IOT_COAP_MSG_OPTION_LENGTH_INVALID;
        }

        if (message->option_count < TC_IOT_COAP_MAX_OPTION_COUNT) {
            message->options[message->option_count].number = sum_delta;
            message->options[message->option_count].length = length;

            if (length > 0) {
                message->options[message->option_count].value = buffer+pos;
                pos += length;
            }
            message->option_count++;
        } else {
            TC_IOT_LOG_ERROR("option buffer not enough, skip option(number=%d,length=%d)", sum_delta, length);
            if (length > 0) {
                pos += length;
            }
        }

        if (pos > buffer_len) {
            TC_IOT_LOG_ERROR("message end option(number=%d,length=%d)", sum_delta, length);
            return TC_IOT_COAP_PACK_INVALID;
        }
    }

    if (pos > buffer_len) {
        return TC_IOT_COAP_PACK_INVALID;
    } else if (pos == buffer_len) {
        message->p_payload = NULL;
        message->payload_len = 0;
    } else {
        message->p_payload = buffer+pos;
        message->payload_len = buffer_len - pos;
    }

    return TC_IOT_SUCCESS;
}

int tc_iot_coap_message_add_option(tc_iot_coap_message * message, int option_number, int option_length, void * option_value) {
    int i = 0;

    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);

    if (message->option_count >= TC_IOT_COAP_MAX_OPTION_COUNT) {
        return TC_IOT_COAP_MSG_OPTION_COUNT_TOO_MUCH;
    }

    /* 查找Option 插入位置 */
    for(i = message->option_count; i > 0;i--) {
        if (message->options[i-1].number <= option_number) {
            break;
        }
    }

    if (option_number == COAP_OPTION_URI_PATH) {
        TC_IOT_LOG_TRACE("Add Option Uri-Path(size=%d):%s", option_length, (char *) option_value)
    } else if (option_number == COAP_OPTION_URI_QUERY ) {
        TC_IOT_LOG_TRACE("Add Option Uri-Query(size=%d):%s", option_length, (char *) option_value)
    }

    if (i < message->option_count) {
        memmove(&message->options[i+1], &message->options[i], sizeof(message->options[i]));
    }
    message->options[i].number = option_number;
    message->options[i].length = option_length;
    message->options[i].value = option_value;

    message->option_count++;

    return TC_IOT_SUCCESS;
}

int tc_iot_coap_construct(tc_iot_coap_client* c, tc_iot_coap_client_config* p_client_config) {

    int i;
    int ret;
    tc_iot_network_t* p_network; 
    tc_iot_net_context_init_t netcontext;

#ifdef ENABLE_DTLS
    tc_iot_tls_config_t* p_tls_config;
#endif

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_client_config, TC_IOT_NULL_POINTER);

    memset(c, 0, sizeof(*c));

    p_network = &(c->ipstack);
    memset(p_network, 0, sizeof(tc_iot_network_t));

    c->device_info = p_client_config->device_info;
    c->default_handler = p_client_config->default_handler;
    netcontext.fd = -1;
    netcontext.use_tls = p_client_config->use_tls;
    netcontext.host = p_client_config->host;
    netcontext.port = p_client_config->port;
    if (netcontext.use_tls) {
#ifdef ENABLE_DTLS
        p_tls_config = &(netcontext.tls_config);
        if (netcontext.use_tls) {
            p_tls_config->verify_server = 0;
            p_tls_config->timeout_ms = p_client_config->tls_handshake_timeout_ms;

            p_tls_config->psk = p_client_config->psk;
            p_tls_config->psk_len = p_client_config->psk_len;
            p_tls_config->psk_id = p_client_config->psk_id;
            p_tls_config->psk_id_len = p_client_config->psk_id_len;

            if (g_tc_iot_coap_root_ca_certs && strlen(g_tc_iot_coap_root_ca_certs)) {
                p_tls_config->root_ca_in_mem = g_tc_iot_coap_root_ca_certs;
            } else {
                p_tls_config->root_ca_in_mem = NULL;
            }
            p_tls_config->root_ca_location = p_client_config->p_root_ca;
            p_tls_config->device_cert_location = p_client_config->p_client_crt;
            p_tls_config->device_private_key_location =
                p_client_config->p_client_key;
        }

        tc_iot_hal_dtls_init(p_network, &netcontext);
#else
        TC_IOT_LOG_FATAL("tls network not supported.");
        return TC_IOT_TLS_NOT_SUPPORTED;
#endif
    } else {
        tc_iot_hal_udp_init(p_network, &netcontext);
    }

    /* for (i = 0; i < TC_IOT_MAX_MESSAGE_HANDLERS; ++i) { */
    /*     c->message_handlers[i].topicFilter = 0; */
    /* } */

    c->buf_size = TC_IOT_COAP_SEND_BUF_SIZE;
    c->readbuf_size = TC_IOT_COAP_RECV_BUF_SIZE;
    TC_IOT_LOG_TRACE("mqtt client buf_size=%ld,readbuf_size=%ld,", c->buf_size,
              c->readbuf_size);
    c->next_packetid = 0;

    ret = c->ipstack.do_connect(&(c->ipstack), NULL, 0);
    return ret;
}


static char * _tc_iot_coap_message_id_to_token( unsigned short message_id, char token[TC_IOT_COAP_MAX_TOKEN_LEN])
{
    int i = 0;
    unsigned char temp;
    unsigned char high;
    unsigned char low;
    static const char map_byte_to_hex[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    token[0] = map_byte_to_hex[((message_id>>12)&0xF)];
    token[1] = map_byte_to_hex[((message_id>>8)&0xF)];
    token[2] = map_byte_to_hex[((message_id>>4)&0xF)];
    token[3] = map_byte_to_hex[((message_id)&0xF)];
    return token;
}

int tc_iot_coap_message_set_message_id(tc_iot_coap_message* message, unsigned short message_id) {
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);
    message->message_id = message_id;
    message->header.bits.token_len = 2*sizeof(message_id);
    _tc_iot_coap_message_id_to_token(message_id, message->token);
    return message->message_id;
}

int tc_iot_coap_message_init(tc_iot_coap_message* message) {
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);

    memset(message, 0, sizeof(*message));
    message->header.bits.ver = TC_IOT_COAP_VER;
    return TC_IOT_SUCCESS;
}

int tc_iot_coap_message_set_token(tc_iot_coap_message* message, int token_len, const unsigned char * token) {
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);

    message->header.bits.token_len = token_len;
    if (token_len) {
        memcpy(message->token, token, message->header.bits.token_len);
    }
    return message->header.bits.token_len;
}

int tc_iot_coap_message_set_payload(tc_iot_coap_message* message, int payload_len, unsigned char * payload) {
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);

    message->payload_len = payload_len;
    if (payload_len) {
        message->p_payload = payload;
    }
    return message->payload_len;
}

int tc_iot_coap_message_set_type(tc_iot_coap_message* message, unsigned char type) {
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);

    message->header.bits.type = type;
    return message->header.bits.type;
}

int tc_iot_coap_message_set_code(tc_iot_coap_message* message, unsigned char code) {
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);
    message->code = code;
    return message->code;
}

int tc_iot_coap_send_message(tc_iot_coap_client* c, tc_iot_coap_message* message,
        tc_iot_coap_con_handler callback, int timeout_ms, void * session_context) {
    int ret = 0;
    int data_len = 0;
    int i = 0;
    tc_iot_coap_session * session;

    ret = tc_iot_coap_serialize(c->buf, c->buf_size, message);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("tc_iot_coap_serialize ret = %d", ret);
        return ret;
    }

    if (ret >= c->buf_size) {
        TC_IOT_LOG_ERROR("tc_iot_coap_serialize ret=%d, buffer not enough", ret);
        return ret;
    }

    data_len = ret;

    if (callback) {
        if (timeout_ms > 0) {
            session = tc_iot_coap_session_find_empty(c);
            if (session) {
                tc_iot_coap_session_init(session, message->message_id, callback, timeout_ms, session_context);
            } else {
                TC_IOT_LOG_ERROR("session not enough");
                for (i = 0; i < TC_IOT_COAP_MAX_SESSION_COUNT; i++) {
                    TC_IOT_LOG_TRACE("message_id[%d]=%d",i,c->sessions[i].message_id);
                }
            }
        } else {
            TC_IOT_LOG_ERROR("with callback but timeout_ms invalid=%d", timeout_ms);
        }

    }

    ret = c->ipstack.do_write(&c->ipstack,  c->buf, data_len, timeout_ms);
    return ret;
}

static int _tc_iot_coap_check_expired_session(tc_iot_coap_client *c) {
    int i;
    tc_iot_coap_session * session;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    for (i = 0; i < TC_IOT_COAP_MAX_SESSION_COUNT; i++) {
        session = &(c->sessions[i]);
        if (session->message_id != 0) {
            if (tc_iot_hal_timer_is_expired(&(session->timer))) {
                TC_IOT_LOG_WARN("session:%d expired", session->message_id);
                if (session->handler) {
                    session->handler(c, TC_IOT_COAP_CON_TIMEOUT, NULL, session->session_context);
                } else {
                    TC_IOT_LOG_ERROR("session:%d handler not found", session->message_id);
                }
                tc_iot_coap_session_release(session);
            } else {
                /* TC_IOT_LOG_TRACE("session:%s not expired, left_ms=%d", session->sid, */
                        /* tc_iot_hal_timer_left_ms(&(session->timer))); */
            }
        }
    }
    return TC_IOT_SUCCESS;
}

tc_iot_coap_session * tc_iot_coap_session_find(tc_iot_coap_client * c, unsigned int message_id) {
    int i = 0;
    for (i = 0; i < TC_IOT_COAP_MAX_SESSION_COUNT; i++) {
        if (message_id == c->sessions[i].message_id) {
            return &c->sessions[i];
        }
    }

    return NULL;
}

tc_iot_coap_session * tc_iot_coap_session_find_empty(tc_iot_coap_client * c) {
    return tc_iot_coap_session_find(c, 0);
}


void tc_iot_coap_session_init(tc_iot_coap_session * session, unsigned short message_id, 
        tc_iot_coap_con_handler callback, int timeout_ms, void * session_context) {
    if (!session) {
        return;
    }
    session->session_context = session_context;
    session->message_id = message_id;
    tc_iot_hal_timer_init(&session->timer);
    tc_iot_hal_timer_countdown_ms(&session->timer, timeout_ms);
    session->handler = callback;
}


void tc_iot_coap_session_release(tc_iot_coap_session * session) {
    if (session) {
        memset(session, 0, sizeof(*session));
    }
}

int tc_iot_coap_yield(tc_iot_coap_client * c, int timeout_ms) {
    int rc = TC_IOT_SUCCESS;
    int left_ms = 0;
    tc_iot_coap_message message;
    tc_iot_timer timer;
    tc_iot_coap_session * session = NULL;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer, timeout_ms);

    _tc_iot_coap_check_expired_session(c);

    do {
        rc = c->ipstack.do_read(&c->ipstack, c->readbuf, c->readbuf_size, timeout_ms);
        if (rc > 0) {
            if (rc < (c->readbuf_size-1)) {
                c->readbuf[rc] = '\0';
            } else {
                TC_IOT_LOG_WARN("readbuf fully used(rc=%d), suggest use bigger buffer", rc);
            }
            tc_iot_coap_message_init(&message);
            rc = tc_iot_coap_deserialize(&message, c->readbuf, rc);
            if (TC_IOT_SUCCESS != rc) {
                TC_IOT_LOG_ERROR("tc_iot_coap_deserialize rc = %d", rc);
            } else {
                session = tc_iot_coap_session_find(c, message.message_id);
                if (session && session->handler) {
                    TC_IOT_LOG_TRACE("Response for request mid=%d, Total timer left=%dms", 
                            message.message_id, tc_iot_hal_timer_left_ms(&session->timer));
                    session->handler(c, TC_IOT_COAP_CON_SUCCESS,  &message, session->session_context);
                    tc_iot_coap_session_release(session);
                } else if (c->default_handler) {
                    c->default_handler(c, &message);
                } else {
                    TC_IOT_LOG_ERROR("no handler for message id=%d", message.message_id);
                }
            }
            break;
        }

    } while (!tc_iot_hal_timer_is_expired(&timer));

    return rc;
}

int tc_iot_coap_get_message_payload(tc_iot_coap_message* message, int *payload_len, unsigned char **payload) {
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(payload_len, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(payload, TC_IOT_NULL_POINTER);

    *payload_len = message->payload_len;
    *payload = message->p_payload;
}

unsigned char tc_iot_coap_get_message_code(tc_iot_coap_message* message) {
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);
    return message->code;
}

static void _tc_iot_coap_con_auth_handler(tc_iot_coap_client * c, tc_iot_coap_con_status_e ack_status, 
        tc_iot_coap_message * message , void * session_context) {
    unsigned char * payload = NULL;
    int payload_len;
    unsigned short message_code = 0;

    if (ack_status == TC_IOT_COAP_CON_TIMEOUT) {
        c->auth_state = COAP_AUTH_TIMEOUT;
        TC_IOT_LOG_ERROR("message timeout");
        return;
    }

    message_code = tc_iot_coap_get_message_code(message);
    tc_iot_coap_get_message_payload(message, &payload_len, &payload);
    if (message_code != COAP_CODE_201_CREATED) {
        c->auth_state = COAP_AUTH_FAILED;
        if (payload == NULL) {
            payload = "";
        }
        TC_IOT_LOG_ERROR("auth failed, response coap code=%s,message=%s",
                tc_iot_coap_get_message_code_str(message_code),
                payload
                );
        return ;
    }

    if (message && payload) {
        if (payload_len >= sizeof(c->auth_token)+3) {
            TC_IOT_LOG_TRACE("len=%d,payload=%s, oversize for token", payload_len, payload);
            c->auth_state = COAP_AUTH_FAILED;
            return;
        } else {
            TC_IOT_LOG_TRACE("auth_token_len=%d,auth_token==%s", payload_len, payload);
            tc_iot_hal_snprintf(c->auth_token, sizeof(c->auth_token), "tk=%s", payload);
            c->auth_state = COAP_AUTH_SUCCESS;
            return ;
        }
    } else {
        TC_IOT_LOG_TRACE("[no payload]");
        c->auth_state = COAP_AUTH_FAILED;
        return ;
    }
}

int tc_iot_coap_auth( tc_iot_coap_client * c) {
    const char * uri_path = "auth";
    int ret  = 0;
    int sign_len = 0;
    char sign_out[TC_IOT_HTTP_TOKEN_REQUEST_FORM_LEN];
    long timestamp = 0;
    long nonce = 0;
    tc_iot_coap_message message;
    tc_iot_coap_device_info * p_device_info = &c->device_info;

    timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    nonce = tc_iot_hal_random();

    sign_len = tc_iot_create_auth_request_form(
        sign_out, sizeof(sign_out), p_device_info->secret,
        strlen(p_device_info->secret), p_device_info->client_id,
        strlen(p_device_info->client_id), p_device_info->device_name,
        strlen(p_device_info->device_name), TC_IOT_COAP_AUTH_TOKEN_EXPIRE_TIME, nonce,
        p_device_info->product_id, strlen(p_device_info->product_id),
        timestamp);
    if (sign_len < sizeof(sign_out)) {
        sign_out[sign_len] = '\0';
        TC_IOT_LOG_TRACE("signed request form:\n%s", sign_out);
    } else {
        return TC_IOT_FAILURE;
    }

    tc_iot_coap_message_init(&message);
    tc_iot_coap_message_set_message_id(&message, tc_iot_coap_get_next_pack_id(c));
    tc_iot_coap_message_set_type(&message, COAP_CON);
    tc_iot_coap_message_set_code(&message, COAP_CODE_002_POST);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_PATH, strlen(uri_path), (unsigned char *)uri_path);
    tc_iot_coap_message_set_payload(&message, strlen(sign_out), sign_out);

auth_start:
    c->auth_state = COAP_AUTH_INITIAL;
    ret = tc_iot_coap_send_message(c, &message, _tc_iot_coap_con_auth_handler, TC_IOT_COAP_AUTH_TIMEOUT_MS, NULL);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("send message failed, ret=%d", ret);
    } else {
        TC_IOT_LOG_TRACE("send message success, sent size=%d", ret);
    }

    while (c->auth_state != COAP_AUTH_SUCCESS) {
        tc_iot_coap_yield(c, TC_IOT_COAP_YIELD_TIMEOUT_MS);
        switch(c->auth_state) {
            case COAP_AUTH_TIMEOUT:
                TC_IOT_LOG_ERROR("Auth timeout, retry now.");
                goto auth_start;
            case COAP_AUTH_FAILED:
                TC_IOT_LOG_ERROR("Auth failed, please check config or network.");
                return TC_IOT_FAILURE;
        }
    }

    if (c->auth_state == COAP_AUTH_SUCCESS) {
        c->auth_token_expire_time = timestamp + TC_IOT_COAP_AUTH_TOKEN_EXPIRE_TIME;
    }
    return TC_IOT_SUCCESS;
}


int tc_iot_coap_publish( tc_iot_coap_client * c, const char * uri_path, const char * topic_query_uri, 
        const char * msg, tc_iot_coap_con_handler callback) {
    int ret  = 0;
    tc_iot_coap_message message;

    tc_iot_coap_message_init(&message);
    tc_iot_coap_message_set_message_id(&message, tc_iot_coap_get_next_pack_id(c));
    tc_iot_coap_message_set_type(&message, COAP_CON);
    tc_iot_coap_message_set_code(&message, COAP_CODE_002_POST);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_PATH, strlen(uri_path), (unsigned char *)uri_path);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_QUERY, strlen(c->auth_token), (unsigned char *)c->auth_token);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_QUERY, strlen(topic_query_uri), (unsigned char *)topic_query_uri);
    tc_iot_coap_message_set_payload(&message, strlen(msg), (unsigned char *)msg);
    ret = tc_iot_coap_send_message(c, &message, callback, TC_IOT_COAP_MESSAGE_ACK_TIMEOUT_MS, NULL);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("send message failed, ret=%d", ret);
    } else {
        TC_IOT_LOG_TRACE("send message success, sent size=%d", ret);
    }
    return ret;
}


int tc_iot_coap_rpc( tc_iot_coap_client * c, const char * uri_path, 
        const char * topic_query_uri, const char * topic_resp_uri,
        const char * msg, tc_iot_coap_con_handler callback) {
    int ret  = 0;
    tc_iot_coap_message message;

    tc_iot_coap_message_init(&message);
    tc_iot_coap_message_set_message_id(&message, tc_iot_coap_get_next_pack_id(c));
    tc_iot_coap_message_set_type(&message, COAP_CON);
    tc_iot_coap_message_set_code(&message, COAP_CODE_002_POST);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_PATH, strlen(uri_path), (unsigned char *)uri_path);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_QUERY, strlen(c->auth_token), (unsigned char *)c->auth_token);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_QUERY, strlen(topic_query_uri), (unsigned char *)topic_query_uri);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_QUERY, strlen(topic_resp_uri), (unsigned char *)topic_resp_uri);
    tc_iot_coap_message_set_payload(&message, strlen(msg), (unsigned char *)msg);
    ret = tc_iot_coap_send_message(c, &message, callback, TC_IOT_COAP_MESSAGE_ACK_TIMEOUT_MS, NULL);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("send message failed, ret=%d", ret);
    } else {
        TC_IOT_LOG_TRACE("send message success, sent size=%d", ret);
    }
    return ret;
}

void tc_iot_coap_destroy(tc_iot_coap_client* c) {
    TC_IOT_LOG_TRACE(" destroy called.");
}

