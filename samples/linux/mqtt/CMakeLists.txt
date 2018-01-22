add_executable(demo_auth_mqtt_client demo_auth_mqtt_client.c simple_mqtt_client.c)

if(ENABLE_TLS)
    target_link_libraries(demo_auth_mqtt_client tc_iot_suite tc_iot_common tc_iot_hal mbedtls jsmn MQTTPacketClient)
else()
    target_link_libraries(demo_auth_mqtt_client tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
endif()
