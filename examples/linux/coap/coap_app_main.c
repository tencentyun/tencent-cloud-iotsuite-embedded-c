#include "tc_iot_coap_device_config.h"
#include "tc_iot_export.h"

void parse_command(tc_iot_coap_client_config * config, int argc, char ** argv) ;

static void _tc_iot_coap_con_get_time_handler(tc_iot_coap_client * client, tc_iot_coap_con_status_e ack_status, 
        tc_iot_coap_message * message , void * session_context) {
    unsigned char * payload = NULL;
    int payload_len;

    if (ack_status == TC_IOT_COAP_CON_TIMEOUT) {
        TC_IOT_LOG_ERROR("message timeout");
        return;
    }

    tc_iot_coap_get_message_payload(message, &payload_len, &payload);
    if (message && payload) {
        TC_IOT_LOG_TRACE("len=%d,payload=%s", payload_len, payload);
    } else {
        TC_IOT_LOG_TRACE("[no payload]");
    }
}

void _coap_con_default_handler(void * client, tc_iot_coap_message * message ) {
    unsigned char * payload = NULL;
    int payload_len;
    unsigned short message_code = 0;

    message_code = tc_iot_coap_get_message_code(message);

    tc_iot_coap_get_message_payload(message, &payload_len, &payload);
    if (payload == NULL) {
        payload = "";
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

static void _tc_iot_coap_get_time( tc_iot_coap_client * c) {
    const char * uri_path = "time";
    int ret  = 0;

    tc_iot_coap_message message;

    tc_iot_coap_message_init(&message);
    tc_iot_coap_message_set_message_id(&message, tc_iot_coap_get_next_pack_id(c));
    tc_iot_coap_message_set_type(&message, COAP_CON);
    tc_iot_coap_message_set_code(&message, COAP_CODE_001_GET);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_PATH, strlen(uri_path), (unsigned char *)uri_path);
    ret = tc_iot_coap_send_message(c, &message, _tc_iot_coap_con_get_time_handler, 500, NULL);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("send message return =%d", ret);
    } else {
        TC_IOT_LOG_TRACE("send message return =%d", ret);
    }
}

static void _tc_iot_coap_get_wellknown( tc_iot_coap_client * c) {
    const char * well_known_path = ".well-known";
    const char * core_path = "core";
    int ret  = 0;

    tc_iot_coap_message message;

    tc_iot_coap_message_init(&message);
    tc_iot_coap_message_set_message_id(&message, tc_iot_coap_get_next_pack_id(c));
    tc_iot_coap_message_set_type(&message, COAP_CON);
    tc_iot_coap_message_set_code(&message, COAP_CODE_001_GET);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_PATH, strlen(well_known_path), (unsigned char *)well_known_path);
    tc_iot_coap_message_add_option(&message, COAP_OPTION_URI_PATH, strlen(core_path), (unsigned char *)core_path);
    ret = tc_iot_coap_send_message(c, &message, _tc_iot_coap_con_get_time_handler, 500, NULL);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("send message return =%d", ret);
    } else {
        TC_IOT_LOG_TRACE("send message return =%d", ret);
    }
}

static void _coap_con_rpc_handler(tc_iot_coap_client * client, tc_iot_coap_con_status_e ack_status, 
        tc_iot_coap_message * message , void * session_context) {
    unsigned char * payload = NULL;
    int payload_len;
    unsigned short message_code = 0;

    if (ack_status == TC_IOT_COAP_CON_TIMEOUT) {
        TC_IOT_LOG_ERROR("message timeout");
        return;
    }
    message_code = tc_iot_coap_get_message_code(message);

    tc_iot_coap_get_message_payload(message, &payload_len, &payload);
    if (message_code != COAP_CODE_201_CREATED) {
        if (payload == NULL) {
            payload = "";
        }
        TC_IOT_LOG_ERROR("publish failed, response coap code=%s,message=%s",
                tc_iot_coap_get_message_code_str(message_code),
                payload
                );
        return ;
    }

    if (message && payload) {
        TC_IOT_LOG_TRACE("len=%d,payload=%s", payload_len, payload);
    } else {
        TC_IOT_LOG_TRACE("[no payload]");
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
    char rpc_pub_topic_query_param[128];
    char rpc_sub_topic_query_param[128];
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

    tc_iot_hal_snprintf(rpc_pub_topic_query_param, sizeof(rpc_pub_topic_query_param), TC_IOT_RPC_PUB_TOPIC_PARM_FMT,
            coap_config.device_info.product_id, 
            coap_config.device_info.device_name);
    tc_iot_hal_snprintf(rpc_sub_topic_query_param, sizeof(rpc_sub_topic_query_param), TC_IOT_RPC_SUB_TOPIC_PARM_FMT,
            coap_config.device_info.product_id, 
            coap_config.device_info.device_name);

    while (!stop) {
        // 基于 CoAP 协议上报数据
        tc_iot_coap_publish(&coap_client, TC_IOT_COAP_SERVICE_PUBLISH_PATH, pub_topic_query_param, "{\"method\":\"get\"}", NULL);
        tc_iot_hal_printf("Publish yielding ...\n");
        tc_iot_coap_yield(&coap_client, TC_IOT_COAP_MESSAGE_ACK_TIMEOUT_MS);

        for (i = 5; i > 0; i--) {
            tc_iot_hal_printf("%d ...\n", i);
            tc_iot_hal_sleep_ms(1000);
        }

        // 基于 CoAP 协议的 RPC 调用
        tc_iot_coap_rpc(&coap_client, TC_IOT_COAP_SERVICE_RPC_PATH, rpc_pub_topic_query_param, 
                rpc_sub_topic_query_param, "{\"method\":\"get\"}", _coap_con_rpc_handler);
        tc_iot_hal_printf("Rpc yielding ...\n");
        tc_iot_coap_yield(&coap_client, TC_IOT_COAP_MESSAGE_ACK_TIMEOUT_MS);
        for (i = 5; i > 0; i--) {
            tc_iot_hal_printf("%d ...\n", i);
            tc_iot_hal_sleep_ms(1000);
        }
    }

    return 0;
}


