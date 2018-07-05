
list(APPEND TC_IOT_MQTT_CLIENT_SRCS tc_iot_client.c  tc_iot_mqtt.c)
add_library(tc_iot_mqtt_client STATIC ${TC_IOT_MQTT_CLIENT_SRCS})
target_link_libraries(tc_iot_mqtt_client MQTTPacketClient tc_iot_hal)


