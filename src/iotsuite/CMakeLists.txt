
aux_source_directory(. IOT_MQTT_CLIENT_SRCS)
add_library(tc_iot_suite STATIC ${IOT_MQTT_CLIENT_SRCS})
target_link_libraries(tc_iot_suite MQTTPacketClient tc_iot_hal)


