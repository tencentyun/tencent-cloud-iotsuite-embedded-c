#include <stdio.h>

#include "tc_iot_inc.h"

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

void tc_iot_coap_con_default_handler(void * client, tc_iot_coap_message * message ) {
    TC_IOT_LOG_TRACE("called");
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


#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-7hjcfc6k"
#define TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "mqtt-5ns8xh714"
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID TC_IOT_CONFIG_DEVICE_PRODUCT_KEY "@" TC_IOT_CONFIG_DEVICE_NAME

int main(int argc, char const* argv[])
{
    tc_iot_coap_client coap_client;
    tc_iot_coap_client_config coap_config = {
        {
            /* device info*/
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
        },
        "localhost",
        5683,
        tc_iot_coap_con_default_handler,
        10000,
        0,
        NULL,
        NULL,
        NULL,
    };

    int ret = 0;
    int i = 0;
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    tc_iot_coap_construct(&coap_client, &coap_config);
    tc_iot_coap_auth(&coap_client);
    tc_iot_coap_publish(&coap_client, TC_IOT_COAP_SERVICE_PUBLISH_PATH, "tp=iot-i0ujhadi/device_t1/pub_test", "{\"method\":\"get\"}");

    ret = tc_iot_coap_yield(&coap_client, 2000);
    return 0;
    while (!stop) {
        tc_iot_hal_printf("-------------req-------------------\n");
        _tc_iot_coap_get_time(&coap_client);
        tc_iot_hal_printf("-------------req-------------------\n");

        tc_iot_hal_printf("-------------rsp-------------------\n");
        ret = tc_iot_coap_yield(&coap_client, 2000);
        tc_iot_hal_printf("-------------rsp-------------------\n");

        tc_iot_hal_printf("-------------req-------------------\n");
        _tc_iot_coap_get_wellknown(&coap_client);
        tc_iot_hal_printf("-------------req-------------------\n");
        tc_iot_hal_printf("-------------rsp-------------------\n");
        ret = tc_iot_coap_yield(&coap_client, 2000);
        tc_iot_hal_printf("-------------rsp-------------------\n");
        for (i = 5; i > 0; i--) {
            tc_iot_hal_printf("%d ...\n", i);
            tc_iot_hal_sleep_ms(1000);
        }
    }

    return 0;
}


