
add_executable(advanced_mqtt app_main.c tc_iot_device_logic.c tc_iot_client_config_opts.c)
target_link_libraries(advanced_mqtt tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)


