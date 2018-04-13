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
    const char * token = "cafe";
    const char * uri_path = "time";
    int option_index = 0;
    int ret = 0;
    char buffer[256];
    int buffer_len = sizeof(buffer);

    char resp[256];
    int resp_len = sizeof(resp);

    memset(&message, 0, sizeof(message));
    message.header.bits.ver = TC_IOT_COAP_VER;
    message.header.bits.token_len = strlen(token);
    memcpy(message.token, token, message.header.bits.token_len);
    message.code = COAP_CODE_001_GET;
    message.message_id = 0x1234;

    option_index = message.option_count;
    message.options[option_index].number = COAP_OPTION_URI_PATH;
    message.options[option_index].length = strlen(uri_path);
    message.options[option_index].value = (char *)uri_path;
    message.option_count++;

    ret = tc_iot_coap_serialize(buffer, buffer_len, &message);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("tc_iot_coap_serialize ret = %d", ret);
        return 0;
    }

    ret = net_request("localhost",5683, buffer, ret, resp, resp_len);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("net_request ret = %d", ret);
        return 0;
    }

    memset(&message, 0, sizeof(message));
    ret = tc_iot_coap_deserialize(&message, resp, ret);
    resp[ret] = '\0';
    if (TC_IOT_SUCCESS != ret) {
        TC_IOT_LOG_ERROR("tc_iot_coap_deserialize ret = %d", ret);
    } else {
        if (message.payload_len && message.p_payload) {
            TC_IOT_LOG_INFO("message paylod(%d):%s", message.payload_len, message.p_payload);
        } else {
            TC_IOT_LOG_INFO("message no paylod");
        }
    }
    return 0;
}

