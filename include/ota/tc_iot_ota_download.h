#ifndef TC_IOT_OTA_DOWNLOAD
#define TC_IOT_OTA_DOWNLOAD

// typedef struct _tc_iot_down_helper{
//     void * context;
//
//     tc_iot_md5_t md5_context;
// }tc_iot_download_helper;

int tc_iot_ota_download(const char* api_url, int partial_start, tc_iot_http_download_callback download_callback, const void * context);

#endif /* end of include guard */
