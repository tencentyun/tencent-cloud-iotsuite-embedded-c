add_executable(light app_main.c tc_iot_device_logic.c ../ota/tc_iot_ota_logic.c ../tc_iot_client_config_opts.c)

if(ENABLE_TLS)
    target_link_libraries(light tc_iot_ota tc_iot_suite tc_iot_common tc_iot_hal mbedtls jsmn MQTTPacketClient)
else()
    target_link_libraries(light tc_iot_ota tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
endif()


