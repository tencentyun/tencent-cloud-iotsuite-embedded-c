list(APPEND TC_IOT_SUITE_SRCS tc_iot_shadow.c  tc_iot_shadow_property.c)

add_library(tc_iot_suite STATIC ${TC_IOT_SUITE_SRCS})
target_link_libraries(tc_iot_suite tc_iot_mqtt_client tc_iot_token MQTTPacketClient tc_iot_hal)


