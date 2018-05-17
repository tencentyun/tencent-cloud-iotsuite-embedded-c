
list(APPEND TC_IOT_MQTT_CLIENT_SRCS tc_iot_client.c  tc_iot_mqtt.c tc_iot_token.c)
if (ENABLE_DATA_TEMPLATE)
    list(APPEND TC_IOT_MQTT_CLIENT_SRCS tc_iot_shadow.c tc_iot_shadow_property.c)
endif()

add_library(tc_iot_suite STATIC ${TC_IOT_MQTT_CLIENT_SRCS})
target_link_libraries(tc_iot_suite MQTTPacketClient tc_iot_hal)


