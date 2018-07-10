
list(APPEND TC_IOT_MQTT_TOKEN_SRCS tc_iot_token.c tc_iot_active_device.c)

add_library(tc_iot_token STATIC ${TC_IOT_MQTT_TOKEN_SRCS})
target_link_libraries(tc_iot_token tc_iot_common tc_iot_hal)

add_library(tc_iot_http_mqapi STATIC tc_iot_http_mqapi.c)
target_link_libraries(tc_iot_http_mqapi tc_iot_common tc_iot_hal)


