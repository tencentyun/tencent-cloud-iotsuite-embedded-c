
if (ENABLE_OTA) 
    add_executable(light app_main.c tc_iot_device_logic.c ../ota/tc_iot_ota_logic.c ../tc_iot_client_config_opts.c)
else()
    add_executable(light app_main.c tc_iot_device_logic.c ../tc_iot_client_config_opts.c)
endif()

if(ENABLE_TLS)
    if (ENABLE_OTA) 
        target_link_libraries(light tc_iot_ota tc_iot_suite tc_iot_common tc_iot_hal mbedtls jsmn MQTTPacketClient)
    else()
        target_link_libraries(light tc_iot_suite tc_iot_common tc_iot_hal mbedtls jsmn MQTTPacketClient)
    endif()
else()
    if (ENABLE_OTA) 
        target_link_libraries(light tc_iot_ota tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
    else()
        target_link_libraries(light tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
    endif()
endif()


