
list(APPEND TC_IOT_MQTT_TOKEN_SRCS tc_iot_token.c)

add_library(tc_iot_token STATIC ${TC_IOT_MQTT_TOKEN_SRCS})
target_link_libraries(tc_iot_token tc_iot_common MQTTPacketClient tc_iot_hal)


