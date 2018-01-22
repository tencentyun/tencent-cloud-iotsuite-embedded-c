#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_hal_net_read(tc_iot_network_t* network, unsigned char* buffer,
                          int len, int timeout_ms) {
    struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    if (interval.tv_sec < 0 ||
        (interval.tv_sec == 0 && interval.tv_usec <= 0)) {
        interval.tv_sec = 0;
        interval.tv_usec = 100;
    }

    int socket_fd = network->net_context.fd;

    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval,
               sizeof(struct timeval));

    int bytes = 0;
    while (bytes < len) {
        int rc = recv(socket_fd, &buffer[bytes], (size_t)(len - bytes), 0);
        if (rc == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) bytes = -1;
            break;
        } else if (rc == 0) {
            bytes = 0;
            break;
        } else {
            bytes += rc;
        }
    }
    return bytes;
}

int tc_iot_hal_net_write(tc_iot_network_t* network, unsigned char* buffer,
                           int len, int timeout_ms) {
    struct timeval tv;

    int socket_fd = network->net_context.fd;

    tv.tv_sec = 0;
    tv.tv_usec = timeout_ms * 1000;

    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv,
               sizeof(struct timeval));
    int rc = write(socket_fd, buffer, len);
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

int tc_iot_hal_net_connect(tc_iot_network_t* network, char* host,
                             uint16_t port) {
    int type = SOCK_STREAM;
    struct sockaddr_in address;
    int rc = -1;
    sa_family_t family = AF_INET;
    struct addrinfo* result = NULL;

    if ((rc = net_prepare()) != 0) {
        return rc;
    }

    struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,
                             0, NULL,      NULL,        NULL};
    if (host) {
        network->net_context.host = host;
    }

    if (port) {
        network->net_context.port = port;
    }

    if ((rc = getaddrinfo(network->net_context.host, NULL, &hints, &result)) ==
        0) {
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
    }

    if (rc == 0) {
        network->net_context.fd = socket(family, type, 0);
        if (network->net_context.fd != -1) {
            rc = connect(network->net_context.fd, (struct sockaddr*)&address,
                         sizeof(address));
            if (rc == 0) {
                network->net_context.is_connected = 1;
            } else {
                LOG_ERROR("tcp connect return error: code=%d, msg=%s", errno, strerror(errno));
                rc = TC_IOT_NET_CONNECT_FAILED;
            }
        }
    }

    return rc;
}

int tc_iot_hal_net_is_connected(tc_iot_network_t* network)
{
    return network->net_context.is_connected; 
}

int tc_iot_hal_net_disconnect(tc_iot_network_t* network) {
    LOG_TRACE("network disconnecting...");
    close(network->net_context.fd);
    network->is_connected = 0;
    LOG_TRACE("network disconnected");
    return TC_IOT_SUCCESS;
}

int tc_iot_hal_net_destroy(tc_iot_network_t* network) {

    if (tc_iot_hal_net_is_connected(network)) {
        tc_iot_hal_net_disconnect(network);
    }

    LOG_TRACE("network destroying...");
    LOG_TRACE("network destroied...");
}

int tc_iot_hal_net_init(tc_iot_network_t* network,
                    tc_iot_net_context_t* net_context) {
    if (NULL == network) {
        return TC_IOT_NETWORK_PTR_NULL;
    }

    network->do_read = tc_iot_hal_net_read;
    network->do_write = tc_iot_hal_net_write;
    network->do_connect = tc_iot_hal_net_connect;
    network->do_disconnect = tc_iot_hal_net_disconnect;
    network->is_connected = tc_iot_hal_net_is_connected;
    network->do_destroy = tc_iot_hal_net_destroy;
    network->net_context = (*net_context);
}

#ifdef __cplusplus
}
#endif
