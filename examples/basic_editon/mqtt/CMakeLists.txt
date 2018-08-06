if (ENABLE_MQTT)
    add_executable(demo_mqtt demo_mqtt.c tc_iot_client_config_opts.c)
    target_link_libraries(demo_mqtt tc_iot_mqtt_client  tc_iot_token tc_iot_common tc_iot_hal jsmn MQTTPacketClient)

    add_executable(demo_custom_topic demo_custom_topic.c tc_iot_client_config_opts.c)
    target_link_libraries(demo_custom_topic tc_iot_mqtt_client tc_iot_token tc_iot_common tc_iot_hal jsmn MQTTPacketClient)

endif()

# if(ENABLE_COAP)
#     ADD_SUBDIRECTORY(coap)
# endif()

