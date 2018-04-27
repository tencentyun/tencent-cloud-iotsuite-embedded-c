#include <stdio.h>

#include "tc_iot_inc.h"

void tc_iot_coap_con_default_handler(void * client, tc_iot_coap_message * message ) {
    TC_IOT_LOG_TRACE("called");
}

int main(int argc, char const* argv[])
{
    /* printf("COAP_CODE_205_CONTENT %d,0x%x\n", (int)COAP_CODE_205_CONTENT,(int)COAP_CODE_205_CONTENT); */
    tc_iot_coap_message message;
    tc_iot_coap_client coap_client;
    tc_iot_coap_client_config coap_config = {
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


