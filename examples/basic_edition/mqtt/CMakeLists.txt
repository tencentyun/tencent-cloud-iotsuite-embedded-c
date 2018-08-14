if (ENABLE_MQTT)
    add_executable(basic_mqtt demo_mqtt.c tc_iot_client_config_opts.c)
    target_link_libraries(basic_mqtt tc_iot_mqtt_client  tc_iot_token tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
endif()

