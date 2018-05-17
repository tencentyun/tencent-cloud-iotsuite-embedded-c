#include "tc_iot_inc.h"

int main(int argc, char const* argv[])
{
    tc_iot_network_t network;
    tc_iot_network_t* p_network;
    tc_iot_net_context_init_t netcontext;

#ifdef ENABLE_DTLS
    tc_iot_tls_config_t* p_tls_config;
#endif

    p_network = &network;
    memset(p_network, 0, sizeof(tc_iot_network_t));

    netcontext.fd = -1;
    netcontext.use_tls = 1;
    netcontext.host = "localhost";
    netcontext.port = 4433;
    if (netcontext.use_tls) {
#ifdef ENABLE_DTLS
        p_tls_config = &(netcontext.tls_config);
        if (netcontext.use_tls) {
            p_tls_config->verify_server = 0;
            p_tls_config->timeout_ms = 2000;
            p_tls_config->root_ca_in_mem = g_tc_iot_mqtt_root_ca_certs;
            p_tls_config->root_ca_location = NULL;
            p_tls_config->device_cert_location = NULL;
            p_tls_config->device_private_key_location = NULL;
        }

        tc_iot_hal_dtls_init(p_network, &netcontext);
        TC_IOT_LOG_INFO("dtls network inited.");
#else
        TC_IOT_LOG_FATAL("dtls network not supported.");
        return TC_IOT_TLS_NOT_SUPPORTED;
#endif
    } else {
        tc_iot_hal_udp_init(p_network, &netcontext);
    }
    p_network->do_connect(p_network, NULL, 0);
    const char * str_send = "Hello, some one.";
    int timeout_ms = 2000;
    char buffer[128] = {0};
    int written_len = p_network->do_write(p_network, str_send , strlen(str_send), timeout_ms);
    TC_IOT_LOG_TRACE("request with:\n%s", str_send);
    int read_len = p_network->do_read(p_network, (unsigned char *)buffer, sizeof(buffer), timeout_ms);
    TC_IOT_LOG_TRACE("response with:\n%s", buffer);

    p_network->do_disconnect(p_network);
    return 0;
}
