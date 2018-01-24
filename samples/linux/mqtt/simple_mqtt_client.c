#include "tc_iot_export.h"

#include "tc_iot_device_config.h"

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("[s->c] %.*s\n", (int)message->payloadlen, (char*)message->payload);
}

static volatile int stop = 0;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop = 1;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop = 1;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
}

int run_simple_mqtt_client(tc_iot_mqtt_client_config* p_client_config) {
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    const char* sub_topic = SUB_TOPIC;
    const char* pub_topic = PUB_TOPIC;

    tc_iot_mqtt_client client;
    tc_iot_mqtt_client* p_client = &client;
    tc_iot_mqtt_client_construct(p_client, p_client_config);
    int ret = tc_iot_mqtt_client_subscribe(p_client, sub_topic, QOS1,
                                           _on_message_received);
    int timeout = 2000;
    tc_iot_mqtt_client_yield(p_client, timeout);

    while (!stop) {
        char* action_get = "{\"method\":\"get\"}";

        tc_iot_mqtt_message pubmsg;
        memset(&pubmsg, '\0', sizeof(pubmsg));
        pubmsg.payload = action_get;
        pubmsg.payloadlen = strlen(pubmsg.payload);
        pubmsg.qos = QOS1;
        pubmsg.retained = 0;
        pubmsg.dup = 0;
        tc_iot_hal_printf("[c->s] shadow_get\n");
        ret = tc_iot_mqtt_client_publish(p_client, pub_topic, &pubmsg);
        if (TC_IOT_SUCCESS != ret) {
            if (ret != TC_IOT_MQTT_RECONNECT_IN_PROGRESS) {
                tc_iot_hal_printf("publish failed: %d, not reconnect, exiting now\n", ret);
                break;
            } else {
                tc_iot_hal_printf("publish failed client trying to reconnect.\n");
            }
        }

        timeout = 5000;
        tc_iot_mqtt_client_yield(p_client, timeout);
    }

    tc_iot_mqtt_client_disconnect(p_client);
}
