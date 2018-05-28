#ifndef TC_IOT_OTA_MQTT_H
#define TC_IOT_OTA_MQTT_H

#define TC_IOT_OTA_ID_LEN        20
#define TC_IOT_OTA_CODE_LEN      20
#define TC_IOT_OTA_STATUS_LEN    20
#define TC_IOT_OTA_MESSAGE_LEN   40


typedef enum _tc_iot_ota_state_e {
    OTA_INITIALIZED,
    OTA_COMMAND_RECEIVED,
    OTA_VERSION_CHECK,
    OTA_DOWNLOAD,
    OTA_MD5_CHECK,
    OTA_START_UPGRADE,
    OTA_UPGRADING,
    OTA_MAX_STATE,
}tc_iot_ota_state_e;

typedef struct _tc_iot_ota_state_item {
    const char * code;
    const char * status;
}tc_iot_ota_state_item;

typedef struct _tc_iot_ota_handler {
    tc_iot_ota_state_e state;
    char ota_id[TC_IOT_OTA_ID_LEN];
    char firmware_md5[TC_IOT_MD5_DIGEST_SIZE*2+1];
    char firmware_url[TC_IOT_HTTP_MAX_URL_LENGTH];
    char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];
    char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];
    tc_iot_mqtt_client * p_mqtt_client;
}tc_iot_ota_handler;

tc_iot_ota_state_item * tc_iot_ota_get_state_item(tc_iot_ota_state_e state);

int tc_iot_ota_init(tc_iot_ota_handler * ota_handler, tc_iot_mqtt_client * mqtt_client, 
        const char * product_id, const char * device_name);
int tc_iot_ota_format_message(tc_iot_ota_handler * ota_handler, char * buffer, int buffer_len, 
        tc_iot_ota_state_e state, const char * message, int percent);
int tc_iot_ota_send_message(tc_iot_ota_handler * ota_handler, char * message);
int tc_iot_ota_set_ota_id(tc_iot_ota_handler * ota_handler, const char * ota_id);
int tc_iot_ota_report(tc_iot_ota_handler * ota_handler, tc_iot_ota_state_e state, char * message, int percent);

#endif /* end of include guard */
