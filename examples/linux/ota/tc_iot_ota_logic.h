#ifndef TC_IOT_OTA_LOGIC_H
#define TC_IOT_OTA_LOGIC_H
#include "tc_iot_export.h"

#ifndef TC_IOT_FIRM_VERSION
#define TC_IOT_FIRM_VERSION "LINUXV1.0"
#endif

typedef struct _tc_iot_down_helper{
    FILE * fp;
    int percent;
    tc_iot_md5_t md5_context;
    tc_iot_ota_handler * ota_handler;
}tc_iot_download_helper;

extern tc_iot_ota_handler handler;
extern char ota_sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];
extern char ota_pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];
void _on_ota_message_received(tc_iot_message_data* md);
int my_http_download_callback(const void * context, const char * data, int data_len, int offset, int total);
void do_download (const char * download_url, const char * filename, const char * firmware_md5);


#endif /* end of include guard */
