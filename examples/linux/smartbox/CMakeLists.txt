
add_executable(smartbox app_main.c ../tc_iot_client_config_opts.c)

if(ENABLE_TLS)
    target_link_libraries(smartbox tc_iot_suite tc_iot_common tc_iot_hal mbedtls jsmn MQTTPacketClient)
else()
    target_link_libraries(smartbox tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
endif()


