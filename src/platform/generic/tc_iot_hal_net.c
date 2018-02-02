#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_hal_net_read(tc_iot_network_t* network, unsigned char* buffer,
                        int len, int timeout_ms) {
    LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_write(tc_iot_network_t* network, unsigned char* buffer,
                         int len, int timeout_ms) {
    LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

static int net_prepare(void) {
    LOG_ERROR("not implemented");  
    return (0);
}

int tc_iot_hal_net_connect(tc_iot_network_t* network, char* host,
                           uint16_t port) {
    LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_is_connected(tc_iot_network_t* network) {
    LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_disconnect(tc_iot_network_t* network) {
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_destroy(tc_iot_network_t* network) {
    LOG_ERROR("not implemented");  
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
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

    return TC_IOT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
