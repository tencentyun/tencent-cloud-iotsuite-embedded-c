#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_hal_tls_read(tc_iot_network_t* network, unsigned char* buffer,
                        int len, int timeout_ms) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_tls_write(tc_iot_network_t* network, const unsigned char* buffer,
                         int len, int timeout_ms) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_tls_connect(tc_iot_network_t* network, const char* host,
                           uint16_t port) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_tls_is_connected(tc_iot_network_t* network) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_tls_disconnect(tc_iot_network_t* network) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_tls_destroy(tc_iot_network_t* network) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_tls_init(tc_iot_network_t* network,
                        tc_iot_net_context_t* net_context) {
    if (NULL == network) {
        return TC_IOT_NETWORK_PTR_NULL;
    }

    network->do_read = tc_iot_hal_tls_read;
    network->do_write = tc_iot_hal_tls_write;
    network->do_connect = tc_iot_hal_tls_connect;
    network->do_disconnect = tc_iot_hal_tls_disconnect;
    network->is_connected = tc_iot_hal_tls_is_connected;
    network->do_destroy = tc_iot_hal_tls_destroy;

    /*tc_iot_copy_net_context(&(network->net_context), net_context);*/
    network->net_context.is_connected = 0;

    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

#ifdef __cplusplus
}
#endif
