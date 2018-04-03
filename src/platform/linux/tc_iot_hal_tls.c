#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_hal_tls_read(tc_iot_network_t* network, unsigned char* buffer,
                        int len, int timeout_ms) {
    tc_iot_tls_data_t* tls_data = &(network->net_context.tls_data);
    int read_len = 0;
    int ret = 0;
    char err_str[100];
    tc_iot_timer timer;

    TC_IOT_FUNC_ENTRY;

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer, timeout_ms);

    if (timeout_ms == 0) {
        TC_IOT_LOG_WARN("tls read timeout=%d, network maybe hang forever.", timeout_ms);
        timeout_ms = 1;
    }

    mbedtls_ssl_conf_read_timeout(&(tls_data->conf), timeout_ms);
    while (read_len < len) {
        ret = mbedtls_ssl_read(&(tls_data->ssl_context), buffer + read_len,
                               len - read_len);
        if (ret > 0) {
            read_len += ret;
            TC_IOT_LOG_TRACE("ret=%d, read_len=%d", ret, read_len);
        } else if (ret == 0) {
            TC_IOT_LOG_TRACE("server closed connection, read_len = %d", read_len);
            if (read_len >= 0) {
                TC_IOT_FUNC_EXIT_RC(read_len);
            } else {
                TC_IOT_FUNC_EXIT_RC(TC_IOT_NET_READ_ERROR);
            }
        } else if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
                   ret != MBEDTLS_ERR_SSL_WANT_WRITE &&
                   ret != MBEDTLS_ERR_SSL_TIMEOUT) {
            if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == ret) {
            }
            mbedtls_strerror(ret, err_str, sizeof(err_str));
            TC_IOT_LOG_TRACE("mbedtls_ssl_read returned %d/%s", ret, err_str);
            if (read_len > 0) {
                TC_IOT_FUNC_EXIT_RC(read_len);
            } else {
                TC_IOT_FUNC_EXIT_RC(TC_IOT_NET_READ_ERROR);
            }
        } else {
            if (tc_iot_hal_timer_is_expired(&timer)) {
                break;
            } else {
                TC_IOT_LOG_TRACE("tls_read unkownn ret=%d", ret);
            }
        }
    }

    if (read_len == 0) {
        /* TC_IOT_LOG_TRACE("nothing read."); */
        return TC_IOT_NET_NOTHING_READ;
    } else if (read_len != len) {
        return TC_IOT_NET_READ_TIMEOUT;
    }

    return read_len;
}

int tc_iot_hal_tls_write(tc_iot_network_t* network, const unsigned char* buffer,
                         int len, int timeout_ms) {
    int written_len = 0;
    bool is_write_failed = false;
    int ret = 0;
    tc_iot_tls_data_t* tls_data = &(network->net_context.tls_data);
    tc_iot_timer timer;

    TC_IOT_FUNC_ENTRY;

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer, timeout_ms);

    while (written_len < len) {
        ret = mbedtls_ssl_write(&(tls_data->ssl_context), buffer + written_len,
                                len - written_len);
        if (ret > 0) {
            written_len += ret;
            continue;
        } else if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
                   ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            TC_IOT_LOG_ERROR("mbedtls_ssl_write returned %d", ret);
            TC_IOT_FUNC_EXIT_RC(TC_IOT_TLS_SSL_WRITE_FAILED);
        } else {
            if (tc_iot_hal_timer_is_expired(&timer)) {
                TC_IOT_LOG_ERROR("mbedtls_ssl_write timeout");
                TC_IOT_FUNC_EXIT_RC(TC_IOT_TLS_SSL_WRITE_TIMEOUT);
            } else {
            }
        }
    }

    TC_IOT_FUNC_EXIT_RC(written_len);
}

static int net_prepare(void) {
#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
    !defined(EFI32)
    WSADATA wsaData;

    if (wsa_init_done == 0) {
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            return (MBEDTLS_ERR_NET_SOCKET_FAILED);

        wsa_init_done = 1;
    }
#else
#if !defined(EFIX64) && !defined(EFI32)
    signal(SIGPIPE, SIG_IGN);
#endif
#endif
    return (0);
}

int tc_iot_hal_tls_connect(tc_iot_network_t* network, const char* host,
                           uint16_t port) {
    char port_str[6];
    int ret = 0;
    char* pers = "iot_client";
    char info_buf[512];

    if (host) {
        network->net_context.host = (char *)host;
    }

    if (port) {
        network->net_context.port = port;
    }

    tc_iot_hal_snprintf(port_str, sizeof(port_str), "%d",
                        network->net_context.port);

    tc_iot_tls_data_t* tls_data = &(network->net_context.tls_data);
    tc_iot_tls_config_t* tls_config = &(network->net_context.tls_config);

    if ((ret = net_prepare()) != 0) {
        return (ret);
    }

    mbedtls_ssl_init(&(tls_data->ssl_context));
    mbedtls_net_init(&(tls_data->ssl_fd));

    mbedtls_ssl_config_init(&(tls_data->conf));
    mbedtls_ctr_drbg_init(&(tls_data->ctr_drbg));
    mbedtls_entropy_init(&(tls_data->entropy));

    TC_IOT_LOG_TRACE("mbedtls_ctr_drbg_seed running ...");
    if ((ret = mbedtls_ctr_drbg_seed(
             &(tls_data->ctr_drbg), mbedtls_entropy_func, &(tls_data->entropy),
             (const unsigned char*)pers, strlen(pers))) != 0) {
        TC_IOT_LOG_ERROR("mbedtls_ctr_drbg_seed returned %d", ret);
        return TC_IOT_CTR_DRBG_SEED_FAILED;
    }

    TC_IOT_LOG_TRACE("Connecting to %s/%s...", network->net_context.host, port_str);

    if ((ret =
             mbedtls_net_connect(&(tls_data->ssl_fd), network->net_context.host,
                                 port_str, MBEDTLS_NET_PROTO_TCP)) != 0) {
        TC_IOT_LOG_ERROR("mbedtls_net_connect returned %d", ret);
        switch (ret) {
            case MBEDTLS_ERR_NET_UNKNOWN_HOST:
                return TC_IOT_NET_UNKNOWN_HOST;
            case MBEDTLS_ERR_NET_SOCKET_FAILED:
                return TC_IOT_NET_SOCKET_FAILED;
            case MBEDTLS_ERR_NET_CONNECT_FAILED:
            default:
                return TC_IOT_NET_CONNECT_FAILED;
        };
    }
    network->net_context.is_connected = 1;

    ret = mbedtls_net_set_block(&(tls_data->ssl_fd));
    if (ret != 0) {
        TC_IOT_LOG_ERROR("net_set_block returned %d", ret);
        return TC_IOT_TLS_NET_SET_BLOCK_FAILED;
    }
    TC_IOT_LOG_TRACE("mbed tls connect ok");

    TC_IOT_LOG_TRACE("mbedtls_ssl_config_defaults configing...");
    if ((ret = mbedtls_ssl_config_defaults(
             &(tls_data->conf), MBEDTLS_SSL_IS_CLIENT,
             MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        TC_IOT_LOG_ERROR("mbedtls_ssl_config_defaults returned %d", ret);
        return TC_IOT_TLS_SSL_CONFIG_DEFAULTS_FAILED;
    }

    if (tls_config->verify_server) {
        mbedtls_ssl_conf_authmode(&(tls_data->conf),
                                  MBEDTLS_SSL_VERIFY_REQUIRED);
    } else {
        mbedtls_ssl_conf_authmode(&(tls_data->conf),
                                  MBEDTLS_SSL_VERIFY_OPTIONAL);
    }

    mbedtls_ssl_conf_ca_chain(&(tls_data->conf), &(tls_data->cacert), NULL);
    mbedtls_ssl_conf_rng(&(tls_data->conf), mbedtls_ctr_drbg_random,
                         &(tls_data->ctr_drbg));

    if ((ret = mbedtls_ssl_conf_own_cert(
             &(tls_data->conf), &(tls_data->clicert), &(tls_data->pkey))) !=
        0) {
        TC_IOT_LOG_ERROR("mbedtls_ssl_conf_own_cert returned %d", ret);
        return TC_IOT_TLS_SSL_CONF_OWN_CERT_FAILED;
    }

    TC_IOT_LOG_TRACE("timeout_ms=%d", tls_config->timeout_ms);
    mbedtls_ssl_conf_read_timeout(&(tls_data->conf), tls_config->timeout_ms);

    if ((ret = mbedtls_ssl_setup(&(tls_data->ssl_context),
                                 &(tls_data->conf))) != 0) {
        TC_IOT_LOG_ERROR("mbedtls_ssl_setup returned %d", ret);
        return TC_IOT_TLS_SSL_SETUP_FAILED;
    }

    if ((ret = mbedtls_ssl_set_hostname(&(tls_data->ssl_context),
                                        network->net_context.host)) != 0) {
        TC_IOT_LOG_ERROR("mbedtls_ssl_set_hostname returned %d", ret);
        return TC_IOT_TLS_SSL_SET_HOSTNAME_FAILED;
    }
    TC_IOT_LOG_TRACE("SSL state: %d ", tls_data->ssl_context.state);
    mbedtls_ssl_set_bio(&(tls_data->ssl_context), &(tls_data->ssl_fd),
                        mbedtls_net_send, NULL, mbedtls_net_recv_timeout);

    TC_IOT_LOG_TRACE("SSL state: %d ", tls_data->ssl_context.state);
    TC_IOT_LOG_TRACE("Performing the SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&(tls_data->ssl_context))) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            TC_IOT_LOG_ERROR("mbedtls_ssl_handshake returned %d\n", ret);
            if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
                TC_IOT_LOG_ERROR(
                    "Unable to verify the server's certificate. "
                    "Either it is invalid,\n"
                    "    or you didn't set ca_file or ca_path "
                    "to an appropriate value.\n"
                    "    Alternatively, you may want to use "
                    "auth_mode=optional for testing purposes.\n");
            }
            return TC_IOT_TLS_SSL_HANDSHAKE_FAILED;
        }
    }

    TC_IOT_LOG_TRACE(
        "Handshake success: protocol=%s,ciphersuite=%s,record_expansion=%d",
        mbedtls_ssl_get_version(&(tls_data->ssl_context)),
        mbedtls_ssl_get_ciphersuite(&(tls_data->ssl_context)),
        mbedtls_ssl_get_record_expansion(&(tls_data->ssl_context)));

    TC_IOT_LOG_TRACE("Verifying peer X.509 certificate...");

    if ((tls_data->flags = mbedtls_ssl_get_verify_result(
                    &(tls_data->ssl_context))) != 0) {
        info_buf[sizeof(info_buf) - 1] = '\0';
        mbedtls_x509_crt_verify_info(info_buf, sizeof(info_buf) - 1, "",
                tls_data->flags);
        TC_IOT_LOG_ERROR("verify info:%s", info_buf);
        if (tls_config->verify_server) {
            ret = TC_IOT_TLS_X509_CRT_VERIFY_FAILED;
        } else {
            TC_IOT_LOG_TRACE(" Server verification skipped");
            ret = TC_IOT_SUCCESS;
        }
    } else {
        TC_IOT_LOG_TRACE("Server verification success");
        ret = TC_IOT_SUCCESS;
    }

    /* #ifdef ENABLE_TC_IOT_LOG_TRACE */
    /* if (mbedtls_ssl_get_peer_cert(&(tls_data->ssl_context)) != NULL) { */
    /* info_buf[sizeof(info_buf) - 1] = '\0'; */
    /* mbedtls_x509_crt_info( */
    /* (char*)info_buf, sizeof(info_buf) - 1, "", */
    /* mbedtls_ssl_get_peer_cert(&(tls_data->ssl_context))); */
    /* TC_IOT_LOG_TRACE("peer cert info:%s\n", info_buf); */
    /* } */
    /* #endif */

    return ret;
}

int tc_iot_hal_tls_is_connected(tc_iot_network_t* network) {
    return network->net_context.is_connected;
}

int tc_iot_hal_tls_disconnect(tc_iot_network_t* network) {
    int ret = 0;
    tc_iot_tls_data_t* tls_data = &(network->net_context.tls_data);
    TC_IOT_LOG_TRACE("network disconnecting...");
    do {
        ret = mbedtls_ssl_close_notify(&(tls_data->ssl_context));
    } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    mbedtls_ssl_free(&(tls_data->ssl_context));
    mbedtls_net_free(&(tls_data->ssl_fd));

    mbedtls_ssl_config_free(&(tls_data->conf));
    mbedtls_ctr_drbg_free(&(tls_data->ctr_drbg));
    mbedtls_entropy_free(&(tls_data->entropy));


    network->net_context.is_connected = 0;
    TC_IOT_LOG_TRACE("network disconnected");
    return TC_IOT_SUCCESS;
}

int tc_iot_hal_tls_destroy(tc_iot_network_t* network) {
    tc_iot_tls_data_t* tls_data = &(network->net_context.tls_data);

    if (tc_iot_hal_tls_is_connected(network)) {
        tc_iot_hal_tls_disconnect(network);
    }

    TC_IOT_LOG_TRACE("network destroying...");

    mbedtls_x509_crt_free(&(tls_data->clicert));
    mbedtls_x509_crt_free(&(tls_data->cacert));
    mbedtls_pk_free(&(tls_data->pkey));
    TC_IOT_LOG_TRACE("network destroied...");
}

int tc_iot_hal_tls_init(tc_iot_network_t* network,
                        tc_iot_net_context_init_t* net_context) {
    int ret = 0;

    if (NULL == network) {
        return TC_IOT_NETWORK_PTR_NULL;
    }

    network->do_read = tc_iot_hal_tls_read;
    network->do_write = tc_iot_hal_tls_write;
    network->do_connect = tc_iot_hal_tls_connect;
    network->do_disconnect = tc_iot_hal_tls_disconnect;
    network->is_connected = tc_iot_hal_tls_is_connected;
    network->do_destroy = tc_iot_hal_tls_destroy;

    tc_iot_copy_net_context(&(network->net_context), net_context);

    network->net_context.is_connected = 0;
    tc_iot_tls_config_t* tls_config = &(network->net_context.tls_config);

    tc_iot_tls_data_t* tls_data = &(network->net_context.tls_data);

    mbedtls_x509_crt_init(&(tls_data->cacert));
    mbedtls_x509_crt_init(&(tls_data->clicert));
    mbedtls_pk_init(&(tls_data->pkey));


    if (tls_config->root_ca_in_mem) {
        TC_IOT_LOG_TRACE("Loading preset root CA cert...");
        ret = mbedtls_x509_crt_parse(&(tls_data->cacert),
                                     tls_config->root_ca_in_mem,
                                     strlen(tls_config->root_ca_in_mem) + 1);
        if (ret < 0) {
            TC_IOT_LOG_ERROR(
                "mbedtls_x509_crt_parse returned %d while parsing root cert ",
                ret);
            return TC_IOT_X509_CRT_PARSE_FILE_FAILED;
        }
        TC_IOT_LOG_TRACE("preset root CA cert load success.");
    }

    if (tls_config->root_ca_location) {
        TC_IOT_LOG_TRACE("Loading extra root CA cert...");
        ret = mbedtls_x509_crt_parse_file(&(tls_data->cacert),
                                          tls_config->root_ca_location);
        if (ret < 0) {
            TC_IOT_LOG_ERROR(
                "mbedtls_x509_crt_parse_file returned %d while parsing root "
                "cert "
                "file: %s",
                ret, tls_config->root_ca_location);
            return TC_IOT_X509_CRT_PARSE_FILE_FAILED;
        }
        TC_IOT_LOG_TRACE("extra root CA cert load success.");
    }

    if (tls_config->device_cert_location) {
        TC_IOT_LOG_TRACE("Loading the client cert and key...");
        ret = mbedtls_x509_crt_parse_file(&(tls_data->clicert),
                                          tls_config->device_cert_location);
        if (ret != 0) {
            TC_IOT_LOG_ERROR(
                "mbedtls_x509_crt_parse_file returned %d while parsing device "
                "cert file: %s",
                ret, tls_config->device_cert_location);
            return TC_IOT_X509_CRT_PARSE_FILE_FAILED;
        }
        TC_IOT_LOG_TRACE("client cert and key load success.");
    }

    if (tls_config->device_private_key_location) {
        ret = mbedtls_pk_parse_keyfile(
            &(tls_data->pkey), tls_config->device_private_key_location, "");
        if (ret != 0) {
            TC_IOT_LOG_ERROR(
                "mbedtls_pk_parse_keyfile returned %d while parsing private "
                "key "
                "file: %s",
                ret, tls_config->device_private_key_location);
            return TC_IOT_PK_PARSE_KEYFILE_FAILED;
        }
    }
    TC_IOT_LOG_TRACE("init tls ok");

    return TC_IOT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
