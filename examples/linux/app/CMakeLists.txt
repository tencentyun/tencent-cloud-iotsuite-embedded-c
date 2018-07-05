
add_executable(iotsuite_app app_main.c tc_iot_device_logic.c ../tc_iot_client_config_opts.c)
target_link_libraries(iotsuite_app tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)


