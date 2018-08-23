list(APPEND TC_IOT_COMMON_SRCS tc_iot_base64.c tc_iot_log.c tc_iot_http_utils.c
    tc_iot_string_utils.c tc_iot_sign_utils.c)

if (ENABLE_MQTT)
    list(APPEND TC_IOT_COMMON_SRCS tc_iot_hmac.c tc_iot_json.c tc_iot_md5.c tc_iot_url.c tc_iot_ya_buffer.c)
elseif(ENABLE_COAP)
    list(APPEND TC_IOT_COMMON_SRCS tc_iot_hmac.c tc_iot_url.c tc_iot_ya_buffer.c)
endif()

if (ENABLE_TLS OR ENABLE_DTLS)
    list(APPEND TC_IOT_COMMON_SRCS tc_iot_certs.c)
endif()

add_library(tc_iot_common STATIC ${TC_IOT_COMMON_SRCS})

target_link_libraries(tc_iot_common jsmn)

