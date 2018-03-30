#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

int tc_iot_hal_net_read(tc_iot_network_t* network, unsigned char* buffer,
                        int len, int timeout_ms) {

		IF_NULL_RETURN(network, TC_IOT_NULL_POINTER);

    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_write(tc_iot_network_t* network, const unsigned char* buffer,
                         int len, int timeout_ms) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_connect(tc_iot_network_t* network, const char* host,
                           uint16_t port) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_is_connected(tc_iot_network_t* network) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_disconnect(tc_iot_network_t* network) {
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
}

int tc_iot_hal_net_destroy(tc_iot_network_t* network) {
    TC_IOT_LOG_ERROR("not implemented");
    return TC_IOT_FUCTION_NOT_IMPLEMENTED;
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
	  return TC_IOT_SUCCESS;
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
