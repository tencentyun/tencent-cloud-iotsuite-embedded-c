#ifndef TC_IOT_TOKEN_0117_1710_H
#define TC_IOT_TOKEN_0117_1710_H

int http_refresh_auth_token(const char* api_url, char* root_ca_path,
                            tc_iot_device_info* p_device_info);

#endif /* end of include guard */
