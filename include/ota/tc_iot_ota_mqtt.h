#ifndef TC_IOT_OTA_MQTT_H
#define TC_IOT_OTA_MQTT_H

#define TC_IOT_OTA_ID_LEN            20
#define TC_IOT_OTA_CODE_LEN          20
#define TC_IOT_OTA_STATUS_LEN        20
#define TC_IOT_OTA_MESSAGE_LEN       40
#define TC_IOT_OTA_MAX_VERSION_LEN   40

#define TC_IOT_OTA_METHOD_UPGRADE          "upgrade"
#define TC_IOT_OTA_METHOD_REPORT_UPGRADE   "report_upgrade"
#define TC_IOT_OTA_METHOD_REPORT_FIRM      "report_firm"

#define TC_IOT_OTA_MESSAGE_SUCCESS      "success"
#define TC_IOT_OTA_MESSAGE_FAILED       "failed"

#define TC_IOT_OTA_MAX_DOWNLOAD_URL_LEN    512


typedef enum _tc_iot_ota_state_e {
    OTA_INITIALIZED,
    OTA_COMMAND_RECEIVED,
    OTA_VERSION_CHECK,
    OTA_DOWNLOAD,
    OTA_MD5_CHECK,
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
    char version[TC_IOT_OTA_MAX_VERSION_LEN];
    char download_url[TC_IOT_OTA_MAX_DOWNLOAD_URL_LEN];

    const char * sub_topic;
    const char * pub_topic;
    tc_iot_mqtt_client * p_mqtt_client;
}tc_iot_ota_handler;

tc_iot_ota_state_item * tc_iot_ota_get_state_item(tc_iot_ota_state_e state);

int tc_iot_ota_construct(tc_iot_ota_handler * ota_handler, tc_iot_mqtt_client * mqtt_client, 
        const char * sub_topic, const char * pub_topic, message_handler ota_msg_callback);
void tc_iot_ota_destroy(tc_iot_ota_handler * ota_handler);

int tc_iot_ota_format_message(tc_iot_ota_handler * ota_handler, char * buffer, int buffer_len, 
        tc_iot_ota_state_e state, const char * message, int percent);
int tc_iot_ota_set_ota_id(tc_iot_ota_handler * ota_handler, const char * ota_id);
int tc_iot_ota_set_state(tc_iot_ota_handler * ota_handler, tc_iot_ota_state_e state);
int tc_iot_ota_report_upgrade(tc_iot_ota_handler * ota_handler, tc_iot_ota_state_e state, char * message, int percent);

bool tc_iot_ota_version_larger(const char * mine_version, const char * their_version);
int tc_iot_ota_send_message(tc_iot_ota_handler * ota_handler, char * message);

#endif /* end of include guard */
