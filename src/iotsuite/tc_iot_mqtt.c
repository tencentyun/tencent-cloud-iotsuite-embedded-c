#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

static void _on_new_message_data(tc_iot_message_data* md, MQTTString* topic,
                                 tc_iot_mqtt_message* message) {
    if (!md) {
        LOG_ERROR("md is null");
        return;
    }

    if (!topic) {
        LOG_ERROR("topic is null");
        return;
    }

    if (!message) {
        LOG_ERROR("topic is null");
        return;
    }

    md->topicName = topic;
    md->message = message;
}

static int _handle_reconnect(tc_iot_mqtt_client* c) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    if (!tc_iot_hal_timer_is_expired(&(c->reconnect_timer))) {
        return TC_IOT_MQTT_RECONNECT_IN_PROGRESS;
    }

    if (!tc_iot_mqtt_get_auto_reconnect(c)) {
        LOG_TRACE("auto_reconnect not enabled.");
        return TC_IOT_MQTT_NETWORK_UNAVAILABLE;
    }

    LOG_TRACE("trying to reconnect.");
    int rc = tc_iot_mqtt_reconnect(c);
    if (rc == TC_IOT_SUCCESS) {
        LOG_TRACE("mqtt reconnect success.");
        return TC_IOT_SUCCESS;
    } else {
        LOG_ERROR("attempt to reconnect failed, errCode: %d", rc);
    }

    if (!(c->reconnect_timeout_ms)) {
        c->reconnect_timeout_ms = TC_IOT_MIN_RECONNECT_WAIT_INTERVAL;
    } else {
        c->reconnect_timeout_ms *= 2;
    }

    if (TC_IOT_MAX_RECONNECT_WAIT_INTERVAL < c->reconnect_timeout_ms) {
        LOG_TRACE("mqtt reconnect timer set to %dms, out of range.",
                  c->reconnect_timeout_ms);
        return TC_IOT_MQTT_RECONNECT_FAILED;
    }

    LOG_TRACE("mqtt reconnect timer set to %dms.", c->reconnect_timeout_ms);
    tc_iot_hal_timer_countdown_ms(&(c->reconnect_timer),
                                  c->reconnect_timeout_ms);

    return TC_IOT_MQTT_RECONNECT_IN_PROGRESS;
}

static int _check_connection(tc_iot_mqtt_client* c) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    if (tc_iot_mqtt_is_connected(c)) {
        return TC_IOT_SUCCESS;
    }
    if (tc_iot_mqtt_get_auto_reconnect(c)) {
        return _handle_reconnect(c);
    }
    return TC_IOT_MQTT_NETWORK_UNAVAILABLE;
}

static int _get_next_pack_id(tc_iot_mqtt_client* c) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    return c->next_packetid =
               (c->next_packetid == TC_IOT_MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}

static int _send_packet(tc_iot_mqtt_client* c, int length,
                        tc_iot_timer* timer) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    int rc = TC_IOT_FAILURE;
    int sent = 0;

    while (sent < length && !tc_iot_hal_timer_is_expired(timer)) {
        rc = c->ipstack.do_write(&(c->ipstack), &c->buf[sent], length,
                                 tc_iot_hal_timer_left_ms(timer));
        if (rc < 0) {
            break;
        }
        sent += rc;
    }

    if (sent == length) {
        tc_iot_hal_timer_countdown_second(&c->last_sent,
                                          c->keep_alive_interval);
        rc = TC_IOT_SUCCESS;
    } else {
        rc = TC_IOT_SEND_PACK_FAILED;
    }
    return rc;
}

int tc_iot_mqtt_init(tc_iot_mqtt_client* c,
                     tc_iot_mqtt_client_config* p_client_config) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_client_config, TC_IOT_NULL_POINTER);

    int i;

    tc_iot_network_t* p_network = &(c->ipstack);
    memset(p_network, 0, sizeof(tc_iot_network_t));

    tc_iot_net_context_t netcontext;
    netcontext.fd = -1;
    netcontext.use_tls = p_client_config->use_tls;
    netcontext.host = p_client_config->host;
    netcontext.port = p_client_config->port;
    if (netcontext.use_tls) {
#ifdef ENABLE_TLS
        tc_iot_tls_config_t* p_tls_config;
        p_tls_config = &(netcontext.tls_config);
        if (netcontext.use_tls) {
            p_tls_config->verify_server = 0;
            p_tls_config->root_ca_in_mem = g_tc_iot_mqtt_root_ca_certs;
            p_tls_config->root_ca_location = p_client_config->p_root_ca;
            p_tls_config->device_cert_location = p_client_config->p_client_crt;
            p_tls_config->device_private_key_location =
                p_client_config->p_client_key;
        }

        tc_iot_hal_tls_init(p_network, &netcontext);
#else
        LOG_CRIT("tls network not supported.");
        return TC_IOT_TLS_NOT_SUPPORTED;
#endif
    } else {
        tc_iot_hal_net_init(p_network, &netcontext);
    }

    for (i = 0; i < TC_IOT_MAX_MESSAGE_HANDLERS; ++i) {
        c->message_handlers[i].topicFilter = 0;
    }

    c->command_timeout_ms = p_client_config->command_timeout_ms;
    c->buf_size = TC_IOT_CLIENT_SEND_BUF_SIZE;
    c->readbuf_size = TC_IOT_CLIENT_READ_BUF_SIZE;
    LOG_TRACE("mqtt client buf_size=%ld,readbuf_size=%ld,", c->buf_size,
              c->readbuf_size);
    c->auto_reconnect = p_client_config->auto_reconnect;
    c->clean_session = p_client_config->clean_session;
    c->default_msg_handler = p_client_config->default_msg_handler;
    c->disconnect_handler = p_client_config->disconnect_handler;
    c->ping_outstanding = 0;
    c->next_packetid = 1;
    tc_iot_hal_timer_init(&c->last_sent);
    tc_iot_hal_timer_init(&c->last_received);
    tc_iot_hal_timer_init(&c->reconnect_timer);

    tc_iot_mqtt_set_state(c, CLIENT_INTIALIAZED);
    int ret = c->ipstack.do_connect(&(c->ipstack), NULL, 0);
    if (TC_IOT_SUCCESS == ret) {
        tc_iot_mqtt_set_state(c, CLIENT_NETWORK_READY);
    }
    return ret;
}

static int decodePacket(tc_iot_mqtt_client* c, int* value, int timeout) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(value, TC_IOT_NULL_POINTER);

    unsigned char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
            rc = MQTTPACKET_READ_ERROR;
            goto exit;
        }
        rc = c->ipstack.do_read(&(c->ipstack), &i, 1, timeout);
        if (rc != 1) {
            goto exit;
        }
        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);
exit:
    return len;
}

static int readPacket(tc_iot_mqtt_client* c, tc_iot_timer* timer) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(timer, TC_IOT_NULL_POINTER);

    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    int rc = c->ipstack.do_read(&(c->ipstack), c->readbuf, 1, tc_iot_hal_timer_left_ms(timer));
    if (rc != 1){
        goto exit;
    }

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, tc_iot_hal_timer_left_ms(timer));
    len += MQTTPacket_encode(
        c->readbuf + 1,
        rem_len); /* put the original remaining length back into the buffer */

    if (rem_len > (c->readbuf_size - len)) {
        LOG_ERROR(
            "buffer not enough: rem_len=%d, readbuf_size=%d, len=%d,"
            " please check TC_IOT_CLIENT_READ_BUF_SIZE",
            rem_len, (int)c->readbuf_size, len);
        rc = TC_IOT_BUFFER_OVERFLOW;
        goto exit;
    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the
     * data */
    if (rem_len > 0 &&
        (rc = c->ipstack.do_read(&(c->ipstack), c->readbuf + len, rem_len,
                                 tc_iot_hal_timer_left_ms(timer)) != rem_len)) {
        rc = 0;
        goto exit;
    }

    header.byte = c->readbuf[0];
    rc = header.bits.type;
    if (c->keep_alive_interval > 0) {
        tc_iot_hal_timer_countdown_second(
            &c->last_received, c->keep_alive_interval);  // record the fact that
                                                         // we have successfully
                                                         // received a packet
    }
exit:
    return rc;
}

// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char* topicFilter, MQTTString* topicName) {
    IF_NULL_RETURN(topicFilter, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topicName, TC_IOT_NULL_POINTER);

    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end) {
        if (*curn == '/' && *curf != '/') break;
        if (*curf != '+' && *curf != '#' && *curf != *curn) break;
        if (*curf ==
            '+') {  // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/') nextpos = ++curn + 1;
        } else if (*curf == '#')
            curn = curn_end - 1;  // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}

int deliverMessage(tc_iot_mqtt_client* c, MQTTString* topicName,
                   tc_iot_mqtt_message* message) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topicName, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);

    int i;
    int rc = TC_IOT_FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < TC_IOT_MAX_MESSAGE_HANDLERS; ++i) {
        if (c->message_handlers[i].topicFilter != 0 &&
            (MQTTPacket_equals(topicName,
                               (char*)c->message_handlers[i].topicFilter) ||
             isTopicMatched((char*)c->message_handlers[i].topicFilter,
                            topicName))) {
            if (c->message_handlers[i].fp != NULL) {
                tc_iot_message_data md;
                _on_new_message_data(&md, topicName, message);
                c->message_handlers[i].fp(&md);
                rc = TC_IOT_SUCCESS;
            }
        }
    }

    if (rc == TC_IOT_FAILURE && c->default_msg_handler != NULL) {
        tc_iot_message_data md;
        _on_new_message_data(&md, topicName, message);
        c->default_msg_handler(&md);
        rc = TC_IOT_SUCCESS;
    }

    return rc;
}

int keepalive(tc_iot_mqtt_client* c) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    int rc = TC_IOT_SUCCESS;

    if (c->keep_alive_interval == 0) {
        goto exit;
    }

    if (tc_iot_hal_timer_is_expired(&c->last_sent) ||
        tc_iot_hal_timer_is_expired(&c->last_received)) {
        if (c->ping_outstanding) {
            rc = TC_IOT_FAILURE;
        } else {
            tc_iot_timer timer;
            tc_iot_hal_timer_init(&timer);
            tc_iot_hal_timer_countdown_ms(&timer, c->command_timeout_ms);
            int len = MQTTSerialize_pingreq(c->buf, c->buf_size);
            if (len > 0 &&
                (rc = _send_packet(c, len, &timer)) == TC_IOT_SUCCESS) {
                c->ping_outstanding = 1;
            }
        }
    }

exit:
    return rc;
}

static void _close_session(tc_iot_mqtt_client* c) {
    if (!c) {
        return;
    }

    c->ping_outstanding = 0;
    if (c->clean_session) {
        int i = 0;
        for (i = 0; i < TC_IOT_MAX_MESSAGE_HANDLERS; ++i) {
            c->message_handlers[i].topicFilter = NULL;
        }
    }
}

int cycle(tc_iot_mqtt_client* c, tc_iot_timer* timer) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(timer, TC_IOT_NULL_POINTER);

    int len = 0;
    int rc = TC_IOT_SUCCESS;

    int packet_type = readPacket(c, timer);

    switch (packet_type) {
        default:
            rc = packet_type;
            if (errno > 0) {
                LOG_TRACE("cycle rc=%d, errno=%d, errstr=%s", rc, errno,
                          strerror(errno));
            }
            goto exit;
        case 0:
            break;
        case CONNACK:
        case PUBACK:
        case SUBACK:
        case UNSUBACK:
            break;
        case PUBLISH: {
            MQTTString topicName;
            tc_iot_mqtt_message msg;
            int intQoS;
            msg.payloadlen = 0;
            if (MQTTDeserialize_publish(
                    &msg.dup, &intQoS, &msg.retained, &msg.id, &topicName,
                    (unsigned char**)&msg.payload, (int*)&msg.payloadlen,
                    c->readbuf, c->readbuf_size) != 1) {
                goto exit;
            }
            msg.qos = (tc_iot_mqtt_qos_e)intQoS;
            deliverMessage(c, &topicName, &msg);
            if (msg.qos != TC_IOT_QOS0) {
                if (msg.qos == TC_IOT_QOS1) {
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0,
                                            msg.id);
                } else if (msg.qos == TC_IOT_QOS2) {
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0,
                                            msg.id);
                }
                if (len <= 0) {
                    rc = TC_IOT_FAILURE;
                } else {
                    rc = _send_packet(c, len, timer);
                }
                if (rc == TC_IOT_FAILURE) {
                    goto exit;
                }
            }
            break;
        }
        case PUBREC:
        case PUBREL: {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf,
                                    c->readbuf_size) != 1) {
                rc = TC_IOT_FAILURE;
            } else if ((len = MQTTSerialize_ack(
                            c->buf, c->buf_size,
                            (packet_type == PUBREC) ? PUBREL : PUBCOMP, 0,
                            mypacketid)) <= 0) {
                rc = TC_IOT_FAILURE;
            } else if ((rc = _send_packet(c, len, timer)) !=
                       TC_IOT_SUCCESS)  // send the PUBREL packet
            {
                LOG_TRACE("_send_packet failed, may network unstable.");
                rc = TC_IOT_FAILURE;  // there was a problem
            }
            if (rc == TC_IOT_FAILURE) {
                goto exit;  // there was a problem
            }
            break;
        }

        case PUBCOMP:
            break;
        case PINGRESP:
            c->ping_outstanding = 0;
            break;
    }

    if (keepalive(c) != TC_IOT_SUCCESS) {
        rc = TC_IOT_FAILURE;
        LOG_TRACE("keepalive failed.");
    }

exit:
    if (rc == TC_IOT_SUCCESS) {
        rc = packet_type;
    } else {
        if (tc_iot_mqtt_is_connected(c)) {
            tc_iot_mqtt_disconnect(c);
        }
        _close_session(c);
    }
    return rc;
}

int tc_iot_mqtt_yield(tc_iot_mqtt_client* c, int timeout_ms) {
    int rc = TC_IOT_SUCCESS;
    int left_ms = 0;
    tc_iot_timer timer;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer, timeout_ms);

    do {
        if (!(tc_iot_mqtt_is_connected(c))) {
            if (c->reconnect_timeout_ms > TC_IOT_MAX_RECONNECT_WAIT_INTERVAL) {
                rc = TC_IOT_MQTT_RECONNECT_TIMEOUT;
                break;
            }
            if (tc_iot_mqtt_get_auto_reconnect(c)) {
                rc = _handle_reconnect(c);
                continue;
            }
        }
        if (cycle(c, &timer) < 0) {
            rc = TC_IOT_FAILURE;
            break;
        }

    } while (!tc_iot_hal_timer_is_expired(&timer));

    if (TC_IOT_SUCCESS != rc && !tc_iot_hal_timer_is_expired(&timer)) {
        left_ms = tc_iot_hal_timer_left_ms(&timer);
        LOG_TRACE("cycle failed ret=%d, sleep over left_ms=%d", rc, left_ms);
        tc_iot_hal_sleep_ms(left_ms);
    }
    return rc;
}

int waitfor(tc_iot_mqtt_client* c, int packet_type, tc_iot_timer* timer) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(timer, TC_IOT_NULL_POINTER);

    int rc = TC_IOT_FAILURE;

    do {
        if (tc_iot_hal_timer_is_expired(timer)) {
            break;  // we timed out
        }
        rc = cycle(c, timer);
    } while (rc != packet_type && rc >= 0);

    return rc;
}

int tc_iot_mqtt_reconnect(tc_iot_mqtt_client* c) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    tc_iot_timer connect_timer;
    int len;
    int rc;
    tc_iot_mqtt_connack_data temp;
    tc_iot_mqtt_connack_data* data = &temp;

    if (tc_iot_mqtt_is_connected(c)) {
        rc = TC_IOT_SUCCESS;
        goto exit;
    }

    tc_iot_mqtt_client_state_e state = tc_iot_mqtt_get_state(c);
    if (CLIENT_INTIALIAZED == state) {
        int ret = c->ipstack.do_connect(&(c->ipstack), NULL, 0);
        if (TC_IOT_SUCCESS == ret) {
            tc_iot_mqtt_set_state(c, CLIENT_NETWORK_READY);
        } else {
            return TC_IOT_NET_CONNECT_FAILED;
        }
    }

    tc_iot_hal_timer_init(&connect_timer);
    tc_iot_hal_timer_countdown_ms(&connect_timer, c->command_timeout_ms);

    tc_iot_hal_timer_countdown_second(&c->last_received,
                                      c->keep_alive_interval);
    if ((len = MQTTSerialize_connect(c->buf, c->buf_size,
                                     &(c->connect_options))) <= 0) {
        goto exit;
    }
    if ((rc = _send_packet(c, len, &connect_timer)) != TC_IOT_SUCCESS) {
        LOG_TRACE("_send_packet failed, may network unstable.");
        goto exit;
    }

    if (waitfor(c, CONNACK, &connect_timer) == CONNACK) {
        data->rc = 0;
        data->sessionPresent = 0;
        if (MQTTDeserialize_connack(&data->sessionPresent, &data->rc,
                                    c->readbuf, c->readbuf_size) == 1) {
            rc = data->rc;
        } else {
            rc = TC_IOT_FAILURE;
        }
    } else {
        LOG_TRACE("waitfor CONNACK timeout");
        rc = TC_IOT_MQTT_WAIT_ACT_TIMEOUT;
    }

exit:
    if (rc == TC_IOT_SUCCESS) {
        LOG_TRACE("mqtt client reconnect success.");
        tc_iot_mqtt_set_state(c, CLIENT_CONNECTED);
        c->ping_outstanding = 0;
    } else if (rc == TC_IOT_SEND_PACK_FAILED ||
               rc == TC_IOT_MQTT_WAIT_ACT_TIMEOUT) {
        // reconnect network layer
        tc_iot_mqtt_disconnect(c);
    }

    return rc;
}

int tc_iot_mqtt_connect_with_results(tc_iot_mqtt_client* c,
                                     MQTTPacket_connectData* options,
                                     tc_iot_mqtt_connack_data* data) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(data, TC_IOT_NULL_POINTER);

    tc_iot_timer connect_timer;
    int rc = TC_IOT_FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;

    if (tc_iot_mqtt_is_connected(c)) {
        rc = TC_IOT_SUCCESS;
        goto exit;
    }

    tc_iot_hal_timer_init(&connect_timer);
    tc_iot_hal_timer_countdown_ms(&connect_timer, c->command_timeout_ms);

    if (!options) {
        options = &default_options;
    }

    c->keep_alive_interval = options->keepAliveInterval;
    c->clean_session = options->cleansession;
    tc_iot_hal_timer_countdown_second(&c->last_received,
                                      c->keep_alive_interval);
    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0) {
        goto exit;
    }
    if ((rc = _send_packet(c, len, &connect_timer)) != TC_IOT_SUCCESS) {
        LOG_TRACE("_send_packet failed, may network unstable.");
        goto exit;
    }

    if (waitfor(c, CONNACK, &connect_timer) == CONNACK) {
        data->rc = 0;
        data->sessionPresent = 0;
        if (MQTTDeserialize_connack(&data->sessionPresent, &data->rc,
                                    c->readbuf, c->readbuf_size) == 1) {
            rc = data->rc;
            switch (rc) {
                case TC_IOT_CONN_PROTOCOL_UNACCEPTABLE:
                    rc = TC_IOT_MQTT_CONNACK_PROTOCOL_UNACCEPTABLE;
                    break;
                case TC_IOT_CONN_CLIENT_ID_INVALID:
                    rc = TC_IOT_MQTT_CONNACK_CLIENT_ID_INVALID;
                    break;
                case TC_IOT_CONN_SERVICE_UNAVAILABLE:
                    rc = TC_IOT_MQTT_CONNACK_SERVICE_UNAVAILABLE;
                    break;
                case TC_IOT_CONN_BAD_USER_OR_PASSWORD:
                    rc = TC_IOT_MQTT_CONNACK_BAD_USER_OR_PASSWORD;
                    break;
                case TC_IOT_CONN_NOT_AUTHORIZED:
                    rc = TC_IOT_MQTT_CONNACK_NOT_AUTHORIZED;
                    break;
                case TC_IOT_CONN_SUCCESS:
                    rc = TC_IOT_SUCCESS;
                    break;
                default:
                    rc = TC_IOT_MQTT_CONNACK_ERROR;
                    break;
            }
        } else {
            rc = TC_IOT_FAILURE;
        }
    } else {
        LOG_TRACE("waitfor CONNACK timeout");
        rc = TC_IOT_MQTT_WAIT_ACT_TIMEOUT;
    }

exit:
    if (rc == TC_IOT_SUCCESS) {
        c->ping_outstanding = 0;
        tc_iot_mqtt_set_state(c, CLIENT_CONNECTED);
    } else if (rc == TC_IOT_SEND_PACK_FAILED ||
               rc == TC_IOT_MQTT_WAIT_ACT_TIMEOUT) {
        // reconnect network layer
        tc_iot_mqtt_disconnect(c);
    }

    return rc;
}

int tc_iot_mqtt_connect(tc_iot_mqtt_client* c,
                        MQTTPacket_connectData* options) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    tc_iot_mqtt_connack_data data;
    return tc_iot_mqtt_connect_with_results(c, options, &data);
}

int tc_iot_mqtt_set_message_handler(tc_iot_mqtt_client* c,
                                    const char* topicFilter,
                                    message_handler msg_handler) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topicFilter, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(msg_handler, TC_IOT_NULL_POINTER);

    int rc = TC_IOT_FAILURE;
    int i = -1;

    for (i = 0; i < TC_IOT_MAX_MESSAGE_HANDLERS; ++i) {
        if (c->message_handlers[i].topicFilter != NULL &&
            strcmp(c->message_handlers[i].topicFilter, topicFilter) == 0) {
            if (msg_handler == NULL) {
                c->message_handlers[i].topicFilter = NULL;
                c->message_handlers[i].fp = NULL;
            }
            rc = TC_IOT_SUCCESS;
            break;
        }
    }
    if (msg_handler != NULL) {
        if (rc == TC_IOT_FAILURE) {
            for (i = 0; i < TC_IOT_MAX_MESSAGE_HANDLERS; ++i) {
                if (c->message_handlers[i].topicFilter == NULL) {
                    rc = TC_IOT_SUCCESS;
                    break;
                }
            }
        }
        if (i < TC_IOT_MAX_MESSAGE_HANDLERS) {
            c->message_handlers[i].topicFilter = topicFilter;
            c->message_handlers[i].fp = msg_handler;
        }
    }
    return rc;
}

int tc_iot_mqtt_subscribe_with_results(tc_iot_mqtt_client* c,
                                       const char* topicFilter,
                                       tc_iot_mqtt_qos_e qos,
                                       message_handler message_handler,
                                       tc_iot_mqtt_suback_data* data) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topicFilter, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(data, TC_IOT_NULL_POINTER);

    int rc = TC_IOT_FAILURE;
    tc_iot_timer timer;
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char*)topicFilter;

    rc = _check_connection(c);
    if (rc != TC_IOT_SUCCESS) {
        goto exit;
    }

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer, c->command_timeout_ms);

    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, _get_next_pack_id(c),
                                  1, &topic, (int*)&qos);
    if (len <= 0) {
        goto exit;
    }
    if ((rc = _send_packet(c, len, &timer)) != TC_IOT_SUCCESS) {
        LOG_TRACE("_send_packet failed, maybe network unstable.");
        goto exit;
    }

    if (waitfor(c, SUBACK, &timer) == SUBACK) {
        int count = 0;
        unsigned short mypacketid;
        data->grantedQoS = TC_IOT_QOS0;
        if (MQTTDeserialize_suback(&mypacketid, 1, &count,
                                   (int*)&data->grantedQoS, c->readbuf,
                                   c->readbuf_size) == 1) {
            if (data->grantedQoS != 0x80) {
                rc = tc_iot_mqtt_set_message_handler(c, topicFilter,
                                                     message_handler);
            }
        }
    } else {
        LOG_TRACE("waitfor SUBACK timeout");
        rc = TC_IOT_MQTT_WAIT_ACT_TIMEOUT;
    }

exit:
    if (rc == TC_IOT_FAILURE) {
        _close_session(c);
    } else if (rc == TC_IOT_SEND_PACK_FAILED ||
               rc == TC_IOT_MQTT_WAIT_ACT_TIMEOUT) {
        if (tc_iot_mqtt_is_connected(c)) {
            tc_iot_mqtt_disconnect(c);
        }
        return _handle_reconnect(c);
    }

    return rc;
}

int tc_iot_mqtt_subscribe(tc_iot_mqtt_client* c, const char* topicFilter,
                          tc_iot_mqtt_qos_e qos,
                          message_handler message_handler) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topicFilter, TC_IOT_NULL_POINTER);
    tc_iot_mqtt_suback_data data;
    return tc_iot_mqtt_subscribe_with_results(c, topicFilter, qos,
                                              message_handler, &data);
}

int tc_iot_mqtt_unsubscribe(tc_iot_mqtt_client* c, const char* topicFilter) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topicFilter, TC_IOT_NULL_POINTER);

    int rc = TC_IOT_FAILURE;
    tc_iot_timer timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char*)topicFilter;
    int len = 0;

    rc = _check_connection(c);
    if (rc != TC_IOT_SUCCESS) {
        goto exit;
    }

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer, c->command_timeout_ms);

    if ((len = MQTTSerialize_unsubscribe(
             c->buf, c->buf_size, 0, _get_next_pack_id(c), 1, &topic)) <= 0) {
        LOG_ERROR("MQTTSerialize_unsubscribe failed.");
        goto exit;
    }

    if ((rc = _send_packet(c, len, &timer)) != TC_IOT_SUCCESS) {
        LOG_TRACE("_send_packet failed, maybe network unstable.");
        goto exit;
    }

    if (waitfor(c, UNSUBACK, &timer) == UNSUBACK) {
        unsigned short mypacketid;
        if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf,
                                     c->readbuf_size) == 1) {
            tc_iot_mqtt_set_message_handler(c, topicFilter, NULL);
        }
    } else {
        LOG_TRACE("waitfor UNSUBACK timeout");
        rc = TC_IOT_MQTT_WAIT_ACT_TIMEOUT;
    }

exit:
    if (rc == TC_IOT_FAILURE) {
        _close_session(c);
    } else if (rc == TC_IOT_SEND_PACK_FAILED ||
               rc == TC_IOT_MQTT_WAIT_ACT_TIMEOUT) {
        if (tc_iot_mqtt_is_connected(c)) {
            tc_iot_mqtt_disconnect(c);
        }
        return _handle_reconnect(c);
    }
    return rc;
}

int tc_iot_mqtt_publish(tc_iot_mqtt_client* c, const char* topicName,
                        tc_iot_mqtt_message* message) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(topicName, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(message, TC_IOT_NULL_POINTER);

    int rc = TC_IOT_FAILURE;
    tc_iot_timer timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char*)topicName;
    int len = 0;

    rc = _check_connection(c);
    if (rc != TC_IOT_SUCCESS) {
        goto exit;
    }

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer, c->command_timeout_ms);

    if (message->qos == TC_IOT_QOS1 || message->qos == TC_IOT_QOS2) {
        message->id = _get_next_pack_id(c);
    }

    len = MQTTSerialize_publish(
        c->buf, c->buf_size, 0, message->qos, message->retained, message->id,
        topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0) {
        LOG_ERROR("MQTTSerialize_publish failed, please check you payload.");
        goto exit;
    }
    if ((rc = _send_packet(c, len, &timer)) != TC_IOT_SUCCESS) {
        LOG_TRACE("_send_packet failed, maybe network unstable.");
        goto exit;
    }

    if (message->qos == TC_IOT_QOS1) {
        if (waitfor(c, PUBACK, &timer) == PUBACK) {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf,
                                    c->readbuf_size) != 1) {
                rc = TC_IOT_FAILURE;
            }
        } else {
            LOG_TRACE("waitfor PUBACK timeout");
            rc = TC_IOT_MQTT_WAIT_ACT_TIMEOUT;
        }
    } else if (message->qos == TC_IOT_QOS2) {
        if (waitfor(c, PUBCOMP, &timer) == PUBCOMP) {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf,
                                    c->readbuf_size) != 1) {
                rc = TC_IOT_FAILURE;
            }
        } else {
            LOG_TRACE("waitfor PUBCOMP timeout");
            rc = TC_IOT_MQTT_WAIT_ACT_TIMEOUT;
        }
    }

exit:
    if (rc == TC_IOT_FAILURE) {
        _close_session(c);
    } else if (rc == TC_IOT_SEND_PACK_FAILED ||
               rc == TC_IOT_MQTT_WAIT_ACT_TIMEOUT) {
        if (tc_iot_mqtt_is_connected(c)) {
            tc_iot_mqtt_disconnect(c);
        }
        return _handle_reconnect(c);
    }
    return rc;
}

int tc_iot_mqtt_is_connected(tc_iot_mqtt_client* client) {
    IF_NULL_RETURN(client, TC_IOT_NULL_POINTER);
    return client->state == CLIENT_CONNECTED;
}

tc_iot_mqtt_client_state_e tc_iot_mqtt_get_state(tc_iot_mqtt_client* client) {
    IF_NULL_RETURN_DATA(client, CLIENT_INVALID);
    return client->state;
}

int tc_iot_mqtt_set_state(tc_iot_mqtt_client* client,
                          tc_iot_mqtt_client_state_e state) {
    IF_NULL_RETURN(client, TC_IOT_NULL_POINTER);
    client->state = state;
    return TC_IOT_SUCCESS;
}

char tc_iot_mqtt_get_auto_reconnect(tc_iot_mqtt_client* c) {
    IF_NULL_RETURN_DATA(c, 0);
    return c->auto_reconnect;
}

int tc_iot_mqtt_set_auto_reconnect(tc_iot_mqtt_client* c, char auto_reconnect) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->auto_reconnect = auto_reconnect;
    return TC_IOT_SUCCESS;
}

int tc_iot_mqtt_disconnect(tc_iot_mqtt_client* c) {
    int rc = TC_IOT_FAILURE;
    tc_iot_timer timer;
    int len = 0;

    if (!tc_iot_mqtt_is_connected(c)) {
        LOG_TRACE("mqtt client already disconnected.");
        return TC_IOT_SUCCESS;
    }

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer, c->command_timeout_ms);

    len = MQTTSerialize_disconnect(c->buf, c->buf_size);
    if (len > 0) {
        rc = _send_packet(c, len, &timer);
    }

    _close_session(c);

    rc = c->ipstack.do_disconnect(&(c->ipstack));
    tc_iot_mqtt_set_state(c, CLIENT_INTIALIAZED);
    if (c->disconnect_handler) {
        c->disconnect_handler(c, NULL);
    }
    return rc;
}

#ifdef __cplusplus
}
#endif
