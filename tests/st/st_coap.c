#include <stdio.h>

#include "tc_iot_inc.h"

int net_request(const char * host, int port, const char * buffer, int buffer_len, char * resp, int max_resp_len) {
    tc_iot_network_t network;
    tc_iot_network_t* p_network;
    tc_iot_net_context_init_t netcontext;

#ifdef ENABLE_TLS
    tc_iot_tls_config_t* p_tls_config;
#endif

    p_network = &network;
    memset(p_network, 0, sizeof(tc_iot_network_t));

    netcontext.fd = -1;
    netcontext.use_tls = 0;
    netcontext.host = (char *)host;
    netcontext.port = port;
    if (netcontext.use_tls) {
#ifdef ENABLE_TLS
        p_tls_config = &(netcontext.tls_config);
        if (netcontext.use_tls) {
            p_tls_config->verify_server = 0;
            p_tls_config->timeout_ms = 2000;
            /* p_tls_config->root_ca_in_mem = g_tc_iot_mqtt_root_ca_certs; */
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
    int timeout_ms = 2000;
    int written_len = p_network->do_write(p_network, buffer , buffer_len, timeout_ms);
    TC_IOT_LOG_TRACE("request len = %d", written_len);
    int read_len = p_network->do_read(p_network, resp, max_resp_len, timeout_ms);
    TC_IOT_LOG_TRACE("response len = %d", read_len);

    p_network->do_disconnect(p_network);
    return read_len;
}
int main(int argc, char const* argv[])
{
    /* printf("COAP_CODE_205_CONTENT %d,0x%x\n", (int)COAP_CODE_205_CONTENT,(int)COAP_CODE_205_CONTENT); */
    tc_iot_coap_message message;
    tc_iot_coap_client coap_client;
    tc_iot_coap_client_config coap_config = {
        "localhost",
        5683,
        10000,
        0,
        NULL,
        NULL,
        NULL,
    };

    int ret = 0;
    const char * token = "cafe";
    const char * uri_path = "time";

    tc_iot_coap_init(&coap_client, &coap_config);

    tc_iot_coap_message_init(&message);
    tc_iot_coap_message_set_message_id(&message, tc_iot_coap_get_next_pack_id(&coap_client));
    tc_iot_coap_message_set_type(&message, COAP_CON);
    tc_iot_coap_message_set_code(&message, COAP_CODE_001_GET);
    tc_iot_coap_message_set_token(&message, strlen(token), token);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_PATH, strlen(uri_path), (unsigned char *)uri_path);
    tc_iot_coap_send_message(&coap_client, &message);

    tc_iot_coap_yield(&coap_client, 2000);
    return 0;
}


