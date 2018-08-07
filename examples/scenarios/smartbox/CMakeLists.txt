
add_executable(scn_smartbox app_main.c tc_iot_client_config_opts.c)
target_link_libraries(scn_smartbox tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)



