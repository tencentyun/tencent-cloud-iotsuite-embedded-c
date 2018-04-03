#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_hal_net_read(tc_iot_network_t* network, unsigned char* buffer,
                        int len, int timeout_ms) {
    int rc; 
    int socket_fd = -1;
    int bytes = 0;

    struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    if (interval.tv_sec < 0 ||
        (interval.tv_sec == 0 && interval.tv_usec <= 0)) {
        interval.tv_sec = 0;
        interval.tv_usec = 100;
    }

    socket_fd = network->net_context.fd;

    rc = setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval,
               sizeof(struct timeval));

    while (bytes < len) {
        rc = recv(socket_fd, &buffer[bytes], (size_t)(len - bytes), 0);
        if (rc == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                bytes = -1;
            }
            /* TC_IOT_LOG_TRACE("recv rc=%d, errno=%d,str=%s,timeout=%d,ts=%d",rc, errno, strerror(errno), timeout_ms, time(NULL)); */
            break;
        } else if (rc == 0) {
            if (bytes >= 0) {
                return bytes;
            } else {
                return TC_IOT_NET_READ_ERROR;
            }
        } else {
            bytes += rc;
        }
    }
    if (bytes == 0) {
        return TC_IOT_NET_NOTHING_READ;
    } else if (bytes != len) {
        return TC_IOT_NET_READ_TIMEOUT;
    }
    return bytes;
}

int tc_iot_hal_net_write(tc_iot_network_t* network, const unsigned char* buffer,
                         int len, int timeout_ms) {
    int rc;
    struct timeval tv;
    int socket_fd = network->net_context.fd;

    /* if (timeout_ms > 0) { */
        /* tv.tv_sec = 0; */
        /* tv.tv_usec = timeout_ms * 1000; */

        /* setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, */
                /* sizeof(struct timeval)); */
    /* } */
    rc = write(socket_fd, buffer, len);
    return rc;
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

int tc_iot_hal_net_connect(tc_iot_network_t* network, const char* host,
                           uint16_t port) {
    int type = SOCK_STREAM;
    struct sockaddr_in address;
    int rc = -1;
    sa_family_t family = AF_INET;
    struct addrinfo* result = NULL;
    struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
                             0, NULL,      NULL,        NULL};

    if ((rc = net_prepare()) != 0) {
        return rc;
    }

    if (host) {
        network->net_context.host = (char *)host;
    }

    if (port) {
        network->net_context.port = port;
    }

    rc = getaddrinfo(network->net_context.host, NULL, &hints, &result);
    if (rc == 0) {
        struct addrinfo* res = result;

        /* prefer ip4 addresses */
        while (res) {
            if (res->ai_family == AF_INET) {
                result = res;
                break;
            }
            res = res->ai_next;
        }

        if (result->ai_family == AF_INET) {
            address.sin_port = htons(network->net_context.port);
            address.sin_family = family = AF_INET;
            address.sin_addr =
                ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
        } else {
            rc = -1;
        }

        freeaddrinfo(result);
    } else {
        TC_IOT_LOG_ERROR("getaddrinfo failed for host:%s, errno=%d,errstr=%s", 
                network->net_context.host,
                rc, gai_strerror(rc)
                );
        rc = TC_IOT_NET_UNKNOWN_HOST;
    }

    if (rc == 0) {
        network->net_context.fd = socket(family, type, 0);
        if (network->net_context.fd != -1) {
            rc = connect(network->net_context.fd, (struct sockaddr*)&address,
                         sizeof(address));
            if (rc == 0) {
                network->net_context.is_connected = 1;
            } else {
                TC_IOT_LOG_ERROR("tcp connect return error: code=%d, msg=%s", errno,
                          strerror(errno));
                rc = TC_IOT_NET_CONNECT_FAILED;
            }
        }
    }

    return rc;
}

int tc_iot_hal_net_is_connected(tc_iot_network_t* network) {
    return network->net_context.is_connected;
}

int tc_iot_hal_net_disconnect(tc_iot_network_t* network) {
    TC_IOT_LOG_TRACE("network disconnecting...");
    close(network->net_context.fd);
    network->is_connected = 0;
    TC_IOT_LOG_TRACE("network disconnected");
    return TC_IOT_SUCCESS;
}

int tc_iot_hal_net_destroy(tc_iot_network_t* network) {
    if (tc_iot_hal_net_is_connected(network)) {
        tc_iot_hal_net_disconnect(network);
    }

    TC_IOT_LOG_TRACE("network destroying...");
    TC_IOT_LOG_TRACE("network destroied...");
    return TC_IOT_SUCCESS;
}

int tc_iot_copy_net_context(tc_iot_net_context_t * net_context, tc_iot_net_context_init_t * init) {
    IF_NULL_RETURN(net_context, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(init, TC_IOT_NULL_POINTER);

    net_context->use_tls           = init->use_tls      ;
    net_context->host              = init->host         ;
    net_context->port              = init->port         ;
    net_context->fd                = init->fd           ;
    net_context->is_connected      = init->is_connected ;
    net_context->extra_context     = init->extra_context;

#ifdef ENABLE_TLS
    net_context->tls_config = init->tls_config;
#endif
}

int tc_iot_hal_net_init(tc_iot_network_t* network,
                        tc_iot_net_context_init_t* net_context) {
    if (NULL == network) {
        return TC_IOT_NETWORK_PTR_NULL;
    }

    network->do_read = tc_iot_hal_net_read;
    network->do_write = tc_iot_hal_net_write;
    network->do_connect = tc_iot_hal_net_connect;
    network->do_disconnect = tc_iot_hal_net_disconnect;
    network->is_connected = tc_iot_hal_net_is_connected;
    network->do_destroy = tc_iot_hal_net_destroy;
    tc_iot_copy_net_context(&(network->net_context), net_context);

    return TC_IOT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
