#include "tc_iot_inc.h"

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
        return number - 12;
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
        TC_IOT_LOG_ERROR("buffer overflow after write code=%d", message->code);
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
            TC_IOT_LOG_ERROR("buffer overflow after write Option number=%d", message->options[i].number);
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
        TC_IOT_LOG_TRACE("request with no payload");
    }

    return pos;
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

    TC_IOT_LOG_TRACE("received: ver=%d,type=%d,tkl=%d,code=%d,message_id=%d",
            message->header.bits.ver, message->header.bits.type,
            message->header.bits.token_len, message->code, message->message_id);

    if (message->header.bits.token_len > 0) {
        memcpy(message->token,buffer+pos, message->header.bits.token_len);
        pos += message->header.bits.token_len;
    }

    if (pos >= buffer_len) {
        TC_IOT_LOG_TRACE("no more data after token");
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
        TC_IOT_LOG_TRACE("delta=%d, length=%d", delta, length);
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
        TC_IOT_LOG_TRACE("option number=%d", sum_delta);

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
