#include "tc_iot_coap_device_config.h"
#include "tc_iot_export.h"

void parse_command(tc_iot_coap_client_config * config, int argc, char ** argv) ;

void _coap_con_default_handler(void * client, tc_iot_coap_message * message ) {
    unsigned char * payload = NULL;
    int payload_len;
    unsigned short message_code = 0;

    message_code = tc_iot_coap_get_message_code(message);

    tc_iot_coap_get_message_payload(message, &payload_len, &payload);
    if (payload == NULL) {
        payload = (unsigned char *)"";
    }

    TC_IOT_LOG_TRACE("response coap code=%s,payload_len=%d,message=%s",
            tc_iot_coap_get_message_code_str(message_code),
            payload_len,
            payload
            );
}


/* 循环退出标识 */
volatile int stop = 0;
void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop ++;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop ++;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
    if (stop >= 3) {
        tc_iot_hal_printf("SIGINT/SIGTERM received over %d times, force shutdown now.\n", stop);
        exit(0);
    }
}



int main(int argc, char * argv[])
{
    tc_iot_coap_client coap_client;
    tc_iot_coap_client_config coap_config = {
        {
            /* device info*/
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
        },
        TC_IOT_CONFIG_COAP_SERVER_HOST,
        TC_IOT_CONFIG_COAP_SERVER_PORT,
        _coap_con_default_handler,
        TC_IOT_CONFIG_DTLS_HANDSHAKE_TIMEOUT_MS,
        TC_IOT_CONFIG_USE_DTLS,
#if defined(ENABLE_DTLS)
        TC_IOT_COAP_DTLS_PSK,
        sizeof(TC_IOT_COAP_DTLS_PSK)-1,
        TC_IOT_COAP_DTLS_PSK_ID,
        sizeof(TC_IOT_COAP_DTLS_PSK_ID) -1,
        NULL,
        NULL,
        NULL,
#endif
    };

    int ret = 0;
    int i = 0;
    char pub_topic_query_param[128];

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    setbuf(stdout, NULL);

    /* 解析命令行参数 */
    parse_command(&coap_config, argc, argv);

    tc_iot_hal_printf("CoAP Server: %s:%d\n", coap_config.host,coap_config.port);
    tc_iot_coap_construct(&coap_client, &coap_config);
    ret = tc_iot_coap_auth(&coap_client);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("CoAP auth failed, ret=%d.\n", ret);
        return 0;
    }

    tc_iot_hal_snprintf(pub_topic_query_param, sizeof(pub_topic_query_param), TC_IOT_PUB_TOPIC_PARM_FMT,
            coap_config.device_info.product_id, 
            coap_config.device_info.device_name);

    while (!stop) {
        // 基于 CoAP 协议上报数据
        tc_iot_coap_publish(&coap_client, TC_IOT_COAP_SERVICE_PUBLISH_PATH, pub_topic_query_param, "{\"method\":\"get\"}", NULL);
        tc_iot_hal_printf("Publish yielding ...\n");
        tc_iot_coap_yield(&coap_client, TC_IOT_COAP_MESSAGE_ACK_TIMEOUT_MS);

        for (i = 20; i > 0; i--) {
            tc_iot_hal_printf("%d ...", i);
            tc_iot_hal_sleep_ms(1000);
        }
        tc_iot_hal_printf("\n");
    }

    return 0;
}


